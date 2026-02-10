# 🛠️ Техническое руководство по реализации

## 📋 Обзор архитектуры

Полное техническое руководство по внутренней архитектуре ESP32 T-Display TOTP включающее систему безопасности, управление памятью, оптимизации и best practices разработки.

## 🔐 Secure Layer Architecture

### Компоненты системы безопасности

```cpp
// Основные менеджеры безопасности
SecureLayerManager      // XOR шифрование + ECDH key exchange
MethodTunnelingManager  // Туннелирование HTTP методов
URLObfuscationManager   // Динамическая обфускация путей  
HeaderObfuscationManager // Маскировка HTTP заголовков
TrafficObfuscationManager // Генерация шумового трафика
```

### ECDH Key Exchange Protocol

#### Client Side (JavaScript)
```javascript
class SecureClient {
    async establishSoftwareSecureConnection() {
        // 1. Генерация ECDH P-256 ключевой пары
        this.keyPair = await window.crypto.subtle.generateKey(
            { name: "ECDH", namedCurve: "P-256" },
            false,
            ["deriveBits", "deriveKey"]
        );
        
        // 2. Экспорт публичного ключа
        const publicKeyBuffer = await window.crypto.subtle.exportKey(
            "raw", this.keyPair.publicKey
        );
        
        // 3. Отправка key exchange запроса
        const response = await fetch('/api/secure/keyexchange', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                pubkey: Array.from(new Uint8Array(publicKeyBuffer))
                    .map(b => b.toString(16).padStart(2, '0')).join(''),
                clientId: this.sessionId
            })
        });
        
        // 4. Обработка ответа сервера
        const data = await response.json();
        await this.processServerPublicKey(data.pubkey);
        
        return true;
    }
    
    async deriveAESKey() {
        // ECDH shared secret computation
        const sharedSecret = await window.crypto.subtle.deriveBits(
            { name: "ECDH", public: this.serverPublicKey },
            this.keyPair.privateKey,
            256
        );
        
        // HKDF для вывода AES ключа
        const keyMaterial = await window.crypto.subtle.importKey(
            "raw", sharedSecret, "HKDF", false, ["deriveKey"]
        );
        
        this.aesKey = await window.crypto.subtle.deriveKey(
            {
                name: "HKDF",
                hash: "SHA-256", 
                salt: new TextEncoder().encode("ESP32-TOTP-Salt"),
                info: new TextEncoder().encode("AES-GCM-Key")
            },
            keyMaterial,
            { name: "AES-GCM", length: 256 },
            false,
            ["encrypt", "decrypt"]
        );
    }
}
```

#### Server Side (C++)
```cpp
class SecureLayerManager {
    bool processProtectedKeyExchange(AsyncWebServerRequest* request, JsonDocument& requestDoc) {
        String clientId = requestDoc["clientId"].as<String>();
        String clientPubKeyHex = requestDoc["pubkey"].as<String>();
        
        // 1. Генерация серверной ECDH ключевой пары
        mbedtls_ecdh_context ctx;
        mbedtls_ecdh_init(&ctx);
        mbedtls_ecdh_setup(&ctx, MBEDTLS_ECP_DP_SECP256R1);
        
        // 2. Генерация случайной ключевой пары
        int ret = mbedtls_ecdh_gen_public(&grp, &d, &Q, mbedtls_ctr_drbg_random, &ctr_drbg);
        if (ret != 0) return false;
        
        // 3. Вычисление shared secret
        mbedtls_mpi shared_secret;
        ret = mbedtls_ecdh_compute_shared(&grp, &shared_secret, &client_public, &d, 
                                         mbedtls_ctr_drbg_random, &ctr_drbg);
        
        // 4. Вывод AES ключа через HKDF
        unsigned char aes_key[32];
        ret = mbedtls_hkdf(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                          salt, salt_len,
                          shared_secret_bytes, shared_secret_len,
                          info, info_len,
                          aes_key, 32);
        
        // 5. Сохранение ключа для клиента
        clientSessions[clientId] = {
            .aesKey = String((char*)aes_key, 32),
            .createdTime = millis(),
            .isValid = true
        };
        
        return true;
    }
}
```

### AES-GCM Encryption Implementation

#### Request Encryption (Client)
```javascript
async encryptRequest(clientId, plaintext) {
    if (!this.aesKey) return null;
    
    // Генерация случайного IV
    const iv = window.crypto.getRandomValues(new Uint8Array(12));
    
    // AES-GCM шифрование
    const encrypted = await window.crypto.subtle.encrypt(
        { name: "AES-GCM", iv: iv },
        this.aesKey,
        new TextEncoder().encode(plaintext)
    );
    
    // Извлечение данных и authentication tag
    const encryptedArray = new Uint8Array(encrypted);
    const data = encryptedArray.slice(0, -16);
    const tag = encryptedArray.slice(-16);
    
    return JSON.stringify({
        type: "secure",
        data: Array.from(data).map(b => b.toString(16).padStart(2, '0')).join(''),
        iv: Array.from(iv).map(b => b.toString(16).padStart(2, '0')).join(''),
        tag: Array.from(tag).map(b => b.toString(16).padStart(2, '0')).join(''),
        counter: this.requestCounter++
    });
}
```

#### Request Decryption (Server)
```cpp
bool decryptRequest(const String& clientId, const String& encryptedJson, String& plaintext) {
    auto session = clientSessions.find(clientId);
    if (session == clientSessions.end()) return false;
    
    JsonDocument doc;
    deserializeJson(doc, encryptedJson);
    
    String dataHex = doc["data"].as<String>();
    String ivHex = doc["iv"].as<String>();  
    String tagHex = doc["tag"].as<String>();
    
    // Конвертация hex в bytes
    unsigned char data[512], iv[12], tag[16];
    hexStringToBytes(dataHex, data);
    hexStringToBytes(ivHex, iv);
    hexStringToBytes(tagHex, tag);
    
    // AES-GCM расшифровка с верификацией
    mbedtls_gcm_context gcm;
    mbedtls_gcm_init(&gcm);
    mbedtls_gcm_setkey(&gcm, MBEDTLS_CIPHER_ID_AES, 
                       (unsigned char*)session->second.aesKey.c_str(), 256);
    
    unsigned char output[512];
    int ret = mbedtls_gcm_auth_decrypt(&gcm, dataHex.length()/2, 
                                      iv, 12, NULL, 0, tag, 16,
                                      data, output);
    
    if (ret == 0) {
        plaintext = String((char*)output, dataHex.length()/2);
        return true;
    }
    
    return false;
}
```

## 🗄️ Memory Management

### Session Management система
```cpp
class SecureLayerManager {
    struct SecureSession {
        String aesKey;
        unsigned long createdTime;
        unsigned long lastActivity;
        bool isValid;
        uint32_t requestCounter;
    };
    
    std::map<String, SecureSession> clientSessions;
    static constexpr unsigned long SESSION_TIMEOUT = 30 * 60 * 1000; // 30 минут
    
    void cleanupExpiredSessions() {
        auto it = clientSessions.begin();
        while (it != clientSessions.end()) {
            if (millis() - it->second.lastActivity > SESSION_TIMEOUT) {
                LOG_INFO("SecureLayer", "Session expired: " + it->first.substring(0,8));
                it = clientSessions.erase(it);
            } else {
                ++it;
            }
        }
    }
}
```

### Memory Optimization принципы
```cpp
// 1. String reserves для избежания реаллокаций
String generateResponse(size_t estimatedSize) {
    String response;
    response.reserve(estimatedSize + 100); // Запас на заголовки
    return response;
}

// 2. Stack-based buffers для временных данных  
void processEncryption() {
    unsigned char buffer[1024]; // Stack allocation
    // ... обработка
    // Автоматическая очистка при выходе из scope
}

// 3. RAII для mbedTLS контекстов
class MbedTLSContext {
    mbedtls_gcm_context gcm;
public:
    MbedTLSContext() { mbedtls_gcm_init(&gcm); }
    ~MbedTLSContext() { mbedtls_gcm_free(&gcm); }
    mbedtls_gcm_context* get() { return &gcm; }
};
```

### Heap Monitoring
```cpp
void logMemoryStats(const String& operation) {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    
    if (freeHeap < 20000) {
        LOG_WARNING("Memory", "Low heap after " + operation + ": " + String(freeHeap));
    }
    
    LOG_DEBUG("Memory", operation + " - Free: " + String(freeHeap) + 
              "b, Min: " + String(minFreeHeap) + "b");
}
```

## 🔄 Persistent Storage Architecture

### LittleFS File Structure
```
/littlefs/
├── config/
│   ├── wifi_config.json      // WiFi credentials
│   ├── admin_config.json     // Admin login hash  
│   ├── pin_config.json       // PIN settings
│   ├── theme_config.json     // UI theme
│   └── startup_mode.json     // Device mode
├── security/
│   ├── device_key.bin        // Unique device encryption key
│   ├── session_data.bin      // Persistent sessions
│   └── url_mappings.json     // Obfuscated URL mappings  
├── data/
│   ├── totp_keys.json.enc    // Encrypted TOTP keys
│   ├── passwords.json.enc    // Encrypted passwords
│   └── boot_counter.dat      // Boot counter for rotation
└── cache/
    ├── ntp_time.dat         // Last NTP sync
    └── battery_stats.json   // Battery usage stats
```

### Encryption-at-Rest система
```cpp
class CryptoManager {
    bool saveEncryptedData(const String& filename, const String& data) {
        // 1. Генерация случайного IV
        unsigned char iv[16];
        esp_fill_random(iv, 16);
        
        // 2. AES-256-CBC шифрование
        mbedtls_aes_context aes;
        mbedtls_aes_init(&aes);
        mbedtls_aes_setkey_enc(&aes, deviceKey, 256);
        
        size_t paddedLength = ((data.length() / 16) + 1) * 16;
        unsigned char* encrypted = new unsigned char[paddedLength];
        
        mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLength,
                             iv, (unsigned char*)data.c_str(), encrypted);
        
        // 3. Сохранение IV + encrypted data
        File file = LittleFS.open(filename, "w");
        file.write(iv, 16);
        file.write(encrypted, paddedLength);
        file.close();
        
        delete[] encrypted;
        return true;
    }
}
```

## 🌐 Web Server Integration

### Async Request Handling
```cpp
class WebServerManager {
    void setupSecureEndpoints() {
        // Unified secure endpoint handler
        server.on("/api/tunnel", HTTP_POST, 
            [this](AsyncWebServerRequest *request) {
                // Пустой handler - основная логика в body handler
            },
            NULL,
            [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, 
                   size_t index, size_t total) {
                
                if (index + len == total) {
                    handleSecureRequest(request, data, len);
                }
            });
    }
    
    void handleSecureRequest(AsyncWebServerRequest* request, uint8_t* data, size_t len) {
        // 1. Authentication check
        if (!isAuthenticated(request)) {
            return request->send(401, "text/plain", "Unauthorized");
        }
        
        // 2. Extract client ID and method
        String clientId = WebServerSecureIntegration::getClientId(request);
        String realMethod = extractRealMethod(request);
        
        // 3. Decrypt request body
        String encryptedBody = String((char*)data, len);
        String decryptedBody;
        
        if (!secureLayer.decryptRequest(clientId, encryptedBody, decryptedBody)) {
            return request->send(400, "text/plain", "Decryption failed");
        }
        
        // 4. Parse target endpoint from decrypted data
        JsonDocument requestData;
        deserializeJson(requestData, decryptedBody);
        String targetEndpoint = requestData["endpoint"].as<String>();
        
        // 5. Route to appropriate handler
        routeSecureRequest(request, targetEndpoint, realMethod, requestData);
    }
}
```

### Response Security Integration
```cpp
void sendSecureResponse(AsyncWebServerRequest* request, int code, 
                       const String& contentType, const String& content,
                       SecureLayerManager& secureLayer) {
    
    String clientId = getClientId(request);
    
    if (clientId.length() > 0 && secureLayer.isSecureSessionValid(clientId)) {
        // Шифрование ответа
        String encryptedResponse;
        if (secureLayer.encryptResponse(clientId, content, encryptedResponse)) {
            request->send(code, "application/json", encryptedResponse);
            return;
        }
    }
    
    // Fallback для незашифрованных клиентов
    request->send(code, contentType, content);
}
```

## ⚡ Performance Optimizations

### Request Batching система
```cpp
class RequestBatcher {
    struct BatchedRequest {
        String endpoint;
        String data;
        unsigned long timestamp;
    };
    
    std::vector<BatchedRequest> pendingRequests;
    static constexpr unsigned long BATCH_TIMEOUT = 50; // 50ms
    
    void addRequest(const String& endpoint, const String& data) {
        pendingRequests.push_back({endpoint, data, millis()});
        
        if (pendingRequests.size() >= MAX_BATCH_SIZE || shouldFlushBatch()) {
            flushBatch();
        }
    }
    
    void flushBatch() {
        if (pendingRequests.empty()) return;
        
        JsonDocument batchDoc;
        JsonArray requests = batchDoc["requests"].to<JsonArray>();
        
        for (const auto& req : pendingRequests) {
            JsonObject reqObj = requests.add<JsonObject>();
            reqObj["endpoint"] = req.endpoint;
            reqObj["data"] = req.data;
        }
        
        processBatchedRequests(batchDoc);
        pendingRequests.clear();
    }
}
```

### Lazy Loading система
```javascript
class LazyLoader {
    static loadedModules = new Set();
    
    static async loadModule(moduleName) {
        if (this.loadedModules.has(moduleName)) {
            return true; // Уже загружен
        }
        
        try {
            switch(moduleName) {
                case 'encryption':
                    await this.loadEncryptionModule();
                    break;
                case 'qr-scanner':
                    await this.loadQRScannerModule();
                    break;
                case 'password-generator':
                    await this.loadPasswordGeneratorModule();
                    break;
            }
            
            this.loadedModules.add(moduleName);
            return true;
        } catch (error) {
            console.error(`Failed to load module ${moduleName}:`, error);
            return false;
        }
    }
}
```

## 🔧 Build System и Configuration

### PlatformIO Configuration
```ini
[env:ttgo-t-display]
platform = espressif32
board = ttgo-t-display
framework = arduino

# Optimization flags
build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DSECURE_LAYER_ENABLED=1
    -DURL_OBFUSCATION_ENABLED=1
    -DHEADER_OBFUSCATION_ENABLED=1
    -DMETHOD_TUNNELING_ENABLED=1
    -DTRAFFIC_OBFUSCATION_ENABLED=1
    -DCACHE_SECURITY_ENABLED=1
    -Os                          # Size optimization
    -ffunction-sections          # Dead code elimination
    -fdata-sections
    -Wl,--gc-sections

# Libraries with specific versions
lib_deps = 
    TFT_eSPI@^2.5.0
    ESPAsyncWebServer@^1.2.3
    AsyncTCP@^1.1.1
    ArduinoJson@^6.21.0
    
# Upload settings
upload_speed = 921600
monitor_speed = 115200
monitor_filters = esp32_exception_decoder
```

### Conditional Compilation система
```cpp
// config.h - Централизованная конфигурация
#ifndef CONFIG_H
#define CONFIG_H

// Security features
#ifndef SECURE_LAYER_ENABLED
#define SECURE_LAYER_ENABLED 1
#endif

#ifndef URL_OBFUSCATION_ENABLED  
#define URL_OBFUSCATION_ENABLED 1
#endif

// Performance settings
#define MAX_CONCURRENT_SESSIONS 10
#define SESSION_TIMEOUT_MS (30 * 60 * 1000)
#define REQUEST_BATCH_SIZE 5
#define CACHE_TTL_MS (5 * 60 * 1000)

// Memory limits
#define MIN_FREE_HEAP 20000
#define MAX_REQUEST_SIZE 8192
#define MAX_RESPONSE_SIZE 16384

#endif
```

## 🧪 Testing и Debugging

### Unit Testing Framework
```cpp
// test/test_security.cpp
#include <unity.h>
#include "secure_layer_manager.h"

void test_ecdh_key_exchange() {
    SecureLayerManager secureLayer;
    
    // Test key generation
    String clientId = "test_client_123";
    JsonDocument keyExchangeReq;
    keyExchangeReq["clientId"] = clientId;
    keyExchangeReq["pubkey"] = "04a1b2c3d4e5f6..."; // Mock public key
    
    bool result = secureLayer.processProtectedKeyExchange(nullptr, keyExchangeReq);
    TEST_ASSERT_TRUE(result);
    
    // Test session creation
    TEST_ASSERT_TRUE(secureLayer.isSecureSessionValid(clientId));
}

void test_aes_encryption() {
    SecureLayerManager secureLayer;
    String clientId = "test_client_123";
    
    // Setup test session
    setupTestSession(clientId);
    
    String plaintext = "Hello, World!";
    String encrypted, decrypted;
    
    TEST_ASSERT_TRUE(secureLayer.encryptResponse(clientId, plaintext, encrypted));
    TEST_ASSERT_TRUE(secureLayer.decryptRequest(clientId, encrypted, decrypted));
    TEST_ASSERT_EQUAL_STRING(plaintext.c_str(), decrypted.c_str());
}

void setUp(void) {
    // Инициализация перед каждым тестом
}

void tearDown(void) {
    // Очистка после каждого теста
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_ecdh_key_exchange);
    RUN_TEST(test_aes_encryption);
    
    return UNITY_END();
}
```

### Production Monitoring
```cpp
class SystemMonitor {
    struct SystemStats {
        uint32_t freeHeap;
        uint32_t minFreeHeap;
        uint32_t activeSessions;
        uint32_t requestsPerMinute;
        float cpuUsage;
    };
    
    void logSystemStats() {
        SystemStats stats = {
            .freeHeap = ESP.getFreeHeap(),
            .minFreeHeap = ESP.getMinFreeHeap(), 
            .activeSessions = secureLayer.getActiveSessionCount(),
            .requestsPerMinute = getRequestRate(),
            .cpuUsage = getCPUUsage()
        };
        
        if (stats.freeHeap < MIN_FREE_HEAP) {
            LOG_WARNING("Monitor", "Low memory: " + String(stats.freeHeap) + "b");
        }
        
        if (stats.activeSessions > MAX_CONCURRENT_SESSIONS) {
            LOG_WARNING("Monitor", "High session count: " + String(stats.activeSessions));
        }
    }
}
```

## 🎯 Best Practices

### Security Best Practices
1. **Всегда проверять authentication** перед обработкой запросов
2. **Использовать RAII** для автоматической очистки ресурсов  
3. **Валидировать все входные данные** включая размеры буферов
4. **Очищать чувствительные данные** из памяти после использования
5. **Логировать security events** но не чувствительные данные

### Performance Best Practices  
1. **Резервировать память** для String объектов заранее
2. **Использовать const references** для передачи больших объектов
3. **Batch операции** где это возможно
4. **Кешировать результаты** вычислений
5. **Мониторить heap usage** в production

### Code Organization Best Practices
1. **Разделение ответственности** - каждый класс решает одну задачу
2. **Dependency Injection** для тестируемости
3. **Interface segregation** - небольшие, специфичные интерфейсы
4. **Error handling** с конкретными error codes
5. **Documentation** для всех public методов

## 🚀 Deployment и Production

### OTA Update система
```cpp
class OTAManager {
    void checkForUpdates() {
        HTTPClient http;
        http.begin("https://api.github.com/repos/user/esp32-totp/releases/latest");
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            JsonDocument doc;
            deserializeJson(doc, payload);
            
            String latestVersion = doc["tag_name"].as<String>();
            if (isNewerVersion(latestVersion, CURRENT_VERSION)) {
                downloadAndInstallUpdate(doc["assets"][0]["download_url"]);
            }
        }
    }
    
    void downloadAndInstallUpdate(const String& downloadUrl) {
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            LOG_ERROR("OTA", "Cannot start update");
            return;
        }
        
        // Download and flash new firmware
        // ... implementation
        
        Update.end(true);
        ESP.restart();
    }
}
```

**Готова архитектура уровня enterprise IoT решения!** 🏗️
