/**
 * @file secure_integration_example.cpp
 * @brief Пример интеграции SecureLayerManager в существующий main.cpp
 * 
 * Этот файл показывает минимальные изменения необходимые для добавления
 * end-to-end шифрования в ваш ESP32 T-Display TOTP проект.
 */

// ========== ДОБАВЛЕНИЯ В СУЩЕСТВУЮЩИЙ main.cpp ==========

// 1. ДОБАВИТЬ ВКЛЮЧЕНИЯ В НАЧАЛО ФАЙЛА
#ifdef SECURE_LAYER_ENABLED
#include "secure_layer_manager.h" 
#include "web_server_secure_integration.h"
#endif

// 2. ДОБАВИТЬ ГЛОБАЛЬНУЮ ПЕРЕМЕННУЮ ПОСЛЕ ДРУГИХ МЕНЕДЖЕРОВ
#ifdef SECURE_LAYER_ENABLED
SecureLayerManager& secureLayerManager = SecureLayerManager::getInstance();
#endif

// 3. ДОБАВИТЬ В ФУНКЦИЮ setup() ПОСЛЕ ИНИЦИАЛИЗАЦИИ CRYPTOMANAGER
void setup() {
    // ... существующий код инициализации ...
    
    // После cryptoManager.begin();
    #ifdef SECURE_LAYER_ENABLED
    LOG_INFO("Main", "Initializing Secure Layer...");
    if (secureLayerManager.begin()) {
        LOG_INFO("Main", "Secure Layer initialized successfully");
    } else {
        LOG_ERROR("Main", "Failed to initialize Secure Layer - continuing without HTTPS");
    }
    #endif
    
    // ... остальной код setup() ...
}

// 4. ДОБАВИТЬ В ФУНКЦИЮ loop() ПОСЛЕ ДРУГИХ UPDATE ВЫЗОВОВ  
void loop() {
    // ... существующий код loop() ...
    
    // После webServerManager.update();
    #ifdef SECURE_LAYER_ENABLED
    secureLayerManager.update();
    #endif
    
    // ... остальной код loop() ...
}

// 5. МОДИФИКАЦИЯ WebServerManager::start() (В web_server.cpp)
void WebServerManager::start() {
    // ... существующий код ...
    
    #ifdef SECURE_LAYER_ENABLED
    // Добавляем secure endpoints
    WebServerSecureIntegration::addSecureEndpoints(server, SecureLayerManager::getInstance());
    LOG_INFO("WebServer", "Secure endpoints added");
    #endif
    
    // ... остальной код start() ...
}

// ========== ПРИМЕР ИСПОЛЬЗОВАНИЯ В API ЭНДПОИНТАХ ==========

// ПРИМЕР МОДИФИКАЦИИ СУЩЕСТВУЮЩЕГО /api/keys ENDPOINT
server.on("/api/keys", HTTP_GET, [this](AsyncWebServerRequest *request){
    if (!isAuthenticated(request)) return request->send(401);
    
    // Проверяем, есть ли заголовок, указывающий на пользовательскую активность
    if (request->hasHeader("X-User-Activity")) {
        resetActivityTimer();
    }
    
    // Генерируем обычный JSON ответ
    JsonDocument doc;
    JsonArray keysArray = doc.to<JsonArray>();
    
    auto keys = keyManager.getAllKeys();
    for (size_t i = 0; i < keys.size(); i++) {
        JsonObject keyObj = keysArray.add<JsonObject>();
        keyObj["name"] = keys[i].name;
        keyObj["code"] = totpGenerator.generateTOTP(keys[i].secret);
        keyObj["timeLeft"] = totpGenerator.getTimeRemaining();
    }
    
    String response;
    serializeJson(doc, response);
    
    // === НОВАЯ ЛОГИКА: АВТОМАТИЧЕСКОЕ ШИФРОВАНИЕ ===
    #ifdef SECURE_LAYER_ENABLED
    WebServerSecureIntegration::sendSecureResponse(request, 200, "application/json", response, SecureLayerManager::getInstance());
    #else
    request->send(200, "application/json", response);
    #endif
});

// ========== HTML СТРАНИЦА С ИНТЕГРАЦИЕЙ SECURE CLIENT ==========

const char* secure_enhanced_html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 T-Display TOTP - Secure</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        .security-indicator {
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
            font-weight: bold;
        }
        .security-none { background: #ffebee; color: #c62828; }
        .security-encrypted { background: #e8f5e8; color: #2e7d32; }
    </style>
    
    <!-- ВКЛЮЧАЕМ SECURE CLIENT -->
    <script src="/secure_client.js"></script>
</head>
<body>
    <h1>🔐 ESP32 T-Display TOTP</h1>
    
    <!-- ИНДИКАТОР БЕЗОПАСНОСТИ -->
    <div id="securityStatus" class="security-indicator security-none">
        🔓 Connecting... Security: None
    </div>
    
    <!-- ОБЫЧНЫЙ ИНТЕРФЕЙС -->
    <div id="totpKeys"></div>
    <button onclick="refreshKeys()">Refresh Keys</button>
    
    <script>
        let secureClient = null;
        
        // Инициализация secure client
        document.addEventListener('DOMContentLoaded', async function() {
            secureClient = initSecureClient(true); // debug mode
            
            // Callback при установке шифрования
            window.onSecureReady = function() {
                document.getElementById('securityStatus').innerHTML = 
                    '🔒 Connected - Security: AES-256-GCM + ECDH';
                document.getElementById('securityStatus').className = 
                    'security-indicator security-encrypted';
                
                // Автоматически обновляем ключи при готовности шифрования
                refreshKeys();
            };
        });
        
        // МОДИФИКАЦИЯ СУЩЕСТВУЮЩЕЙ ФУНКЦИИ refreshKeys()
        async function refreshKeys() {
            try {
                let response;
                
                // Используем secure client если доступен
                if (secureClient && secureClient.isSecure()) {
                    response = await secureClient.makeSecureRequest('/api/keys', {
                        headers: { 'X-User-Activity': 'true' }
                    });
                } else {
                    response = await fetch('/api/keys', {
                        headers: { 'X-User-Activity': 'true' }
                    });
                }
                
                if (response.ok) {
                    const keys = await response.json();
                    displayKeys(keys);
                } else {
                    console.error('Failed to fetch keys:', response.status);
                }
            } catch (error) {
                console.error('Error fetching keys:', error);
            }
        }
        
        function displayKeys(keys) {
            const container = document.getElementById('totpKeys');
            container.innerHTML = '';
            
            keys.forEach(key => {
                const div = document.createElement('div');
                div.innerHTML = `
                    <strong>${key.name}:</strong> 
                    <span style="font-family: monospace; font-size: 1.2em;">${key.code}</span>
                    <small>(${key.timeLeft}s)</small>
                `;
                container.appendChild(div);
            });
        }
        
        // Автообновление каждые 5 секунд
        setInterval(refreshKeys, 5000);
    </script>
</body>
</html>
)";

// ========== ОБСЛУЖИВАНИЕ SECURE CLIENT JS ==========

// ДОБАВИТЬ В WebServerManager::start()
server.on("/secure_client.js", HTTP_GET, [](AsyncWebServerRequest *request){
    // Читаем содержимое файла secure_client.js
    fs::File file = LittleFS.open("/secure_client.js", "r");
    if (file) {
        String content = file.readString();
        file.close();
        AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript", content);
        response->addHeader("Cache-Control", "public, max-age=3600");
        request->send(response);
    } else {
        request->send(404, "text/plain", "secure_client.js not found");
    }
});

// ========== ДИАГНОСТИКА И МОНИТОРИНГ ==========

// ДОБАВИТЬ ДИАГНОСТИЧЕСКИЙ ENDPOINT
server.on("/api/secure/diagnostics", HTTP_GET, [](AsyncWebServerRequest *request){
    #ifdef SECURE_LAYER_ENABLED
    auto& secureLayer = SecureLayerManager::getInstance();
    
    JsonDocument doc;
    doc["secure_layer_enabled"] = true;
    doc["active_sessions"] = secureLayer.getActiveSecureSessionCount();
    doc["client_id"] = WebServerSecureIntegration::getClientId(request);
    doc["session_valid"] = secureLayer.isSecureSessionValid(WebServerSecureIntegration::getClientId(request));
    doc["esp32_free_heap"] = ESP.getFreeHeap();
    doc["esp32_largest_free_block"] = ESP.getMaxAllocHeap();
    #else
    JsonDocument doc;
    doc["secure_layer_enabled"] = false;
    doc["message"] = "Secure layer not compiled";
    #endif
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
});

// ========== MEMORY OPTIMIZATION ==========

// ДОБАВИТЬ В main.cpp ДЛЯ МОНИТОРИНГА ПАМЯТИ
void checkMemoryUsage() {
    static unsigned long lastMemoryCheck = 0;
    if (millis() - lastMemoryCheck > 30000) { // каждые 30 секунд
        size_t freeHeap = ESP.getFreeHeap();
        size_t largestBlock = ESP.getMaxAllocHeap();
        
        #ifdef SECURE_LAYER_ENABLED
        int activeSessions = SecureLayerManager::getInstance().getActiveSecureSessionCount();
        LOG_INFO("Memory", "Free: " + String(freeHeap) + " bytes, Largest: " + String(largestBlock) + 
                          " bytes, Secure sessions: " + String(activeSessions));
        #else
        LOG_INFO("Memory", "Free: " + String(freeHeap) + " bytes, Largest: " + String(largestBlock) + " bytes");
        #endif
        
        // Предупреждение при низкой памяти
        if (freeHeap < 30000) {
            LOG_WARNING("Memory", "Low memory warning! Consider cleaning up sessions.");
            #ifdef SECURE_LAYER_ENABLED
            SecureLayerManager::getInstance().cleanupExpiredSessions();
            #endif
        }
        
        lastMemoryCheck = millis();
    }
}

// ВЫЗВАТЬ В loop()
void loop() {
    // ... существующий код ...
    checkMemoryUsage();
}
