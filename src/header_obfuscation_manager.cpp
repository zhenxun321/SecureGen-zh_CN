#include "header_obfuscation_manager.h"
#include "log_manager.h"
#include <mbedtls/base64.h>
#include <ArduinoJson.h>

HeaderObfuscationManager& HeaderObfuscationManager::getInstance() {
    static HeaderObfuscationManager instance;
    return instance;
}

HeaderObfuscationManager::HeaderObfuscationManager() 
    : initialized(false), enabled(false) {
    stats.totalRequests = 0;
    stats.obfuscatedRequests = 0;
    stats.extractedMetadata = 0;
    stats.lastObfuscatedRequest = 0;
}

HeaderObfuscationManager::~HeaderObfuscationManager() {
    // Clean up resources if needed
}

bool HeaderObfuscationManager::begin() {
    if (initialized) return true;
    
    LOG_INFO("HeaderObfuscation", "Initializing Header Obfuscation Manager...");
    
    // Инициализация маппингов по умолчанию
    initializeDefaultMappings();
    
    initialized = true;
    enabled = true; // По умолчанию включено
    
    LOG_INFO("HeaderObfuscation", "Header obfuscation initialized with " + 
             String(obfuscatedToReal.size()) + " mappings");
    
    return true;
}

void HeaderObfuscationManager::initializeDefaultMappings() {
    // Основные маппинги из тестовой страницы
    obfuscatedToReal["X-Req-UUID"] = "X-Client-ID";
    obfuscatedToReal["X-Security-Level"] = "X-Secure-Request";
    
    // Регистрируем fake headers для игнорирования
    fakeHeaders.push_back("X-Browser-Engine");
    fakeHeaders.push_back("X-Request-Time");
    fakeHeaders.push_back("X-Client-Version");
    fakeHeaders.push_back("X-Feature-Flags");
    fakeHeaders.push_back("X-Session-State");
    
    LOG_DEBUG("HeaderObfuscation", "Default mappings initialized");
}

void HeaderObfuscationManager::setHeaderMapping(const String& obfuscated, const String& real) {
    obfuscatedToReal[obfuscated] = real;
    LOG_DEBUG("HeaderObfuscation", "Mapping added: " + obfuscated + " → " + real);
}

void HeaderObfuscationManager::registerFakeHeader(const String& headerName) {
    fakeHeaders.push_back(headerName);
    LOG_DEBUG("HeaderObfuscation", "Fake header registered: " + headerName);
}

bool HeaderObfuscationManager::hasObfuscatedHeaders(AsyncWebServerRequest* request) {
    if (!request || !enabled) return false;
    
    // Проверяем наличие известных обфусцированных заголовков
    for (const auto& pair : obfuscatedToReal) {
        if (request->hasHeader(pair.first.c_str())) {
            return true;
        }
    }
    
    // Проверяем наличие fake headers
    for (const String& fakeHeader : fakeHeaders) {
        if (request->hasHeader(fakeHeader.c_str())) {
            return true;
        }
    }
    
    return false;
}

bool HeaderObfuscationManager::isHeaderObfuscated(AsyncWebServerRequest* request) {
    if (!initialized || !enabled || !request) return false;
    
    stats.totalRequests++;
    
    bool obfuscated = hasObfuscatedHeaders(request);
    
    if (obfuscated) {
        stats.obfuscatedRequests++;
        stats.lastObfuscatedRequest = millis();
        LOG_DEBUG("HeaderObfuscation", "🎭 Obfuscated request detected");
    }
    
    return obfuscated;
}

String HeaderObfuscationManager::getDeobfuscatedHeader(AsyncWebServerRequest* request, 
                                                       const String& originalHeaderName) {
    if (!request || !enabled) return "";
    
    // Сначала проверяем обфусцированный заголовок
    for (const auto& pair : obfuscatedToReal) {
        if (pair.second == originalHeaderName) {
            // Нашли маппинг: real → obfuscated
            if (request->hasHeader(pair.first.c_str())) {
                String value = request->getHeader(pair.first.c_str())->value();
                LOG_DEBUG("HeaderObfuscation", "🎭 Deobfuscated: " + pair.first + 
                         " → " + originalHeaderName + " = " + value.substring(0, 8) + "...");
                return value;
            }
        }
    }
    
    // Fallback: проверяем оригинальный заголовок (обратная совместимость)
    if (request->hasHeader(originalHeaderName.c_str())) {
        String value = request->getHeader(originalHeaderName.c_str())->value();
        LOG_DEBUG("HeaderObfuscation", "📝 Original header used: " + originalHeaderName);
        return value;
    }
    
    return "";
}

String HeaderObfuscationManager::decodeBase64(const String& encoded) {
    if (encoded.isEmpty()) return "";
    
    // Вычисляем размер буфера для декодирования
    size_t outputLen = 0;
    mbedtls_base64_decode(nullptr, 0, &outputLen, 
                         (const unsigned char*)encoded.c_str(), encoded.length());
    
    if (outputLen == 0) return "";
    
    // Выделяем буфер и декодируем
    unsigned char* buffer = new unsigned char[outputLen + 1];
    int result = mbedtls_base64_decode(buffer, outputLen, &outputLen,
                                      (const unsigned char*)encoded.c_str(), encoded.length());
    
    String decoded = "";
    if (result == 0) {
        buffer[outputLen] = '\0';
        decoded = String((char*)buffer);
    }
    
    delete[] buffer;
    return decoded;
}

String HeaderObfuscationManager::extractEmbeddedMetadata(AsyncWebServerRequest* request) {
    if (!request || !enabled) return "";
    
    // Извлекаем User-Agent
    if (!request->hasHeader("User-Agent")) return "";
    
    String userAgent = request->getHeader("User-Agent")->value();
    
    // Ищем паттерн EdgeInsight/BASE64_DATA
    int startPos = userAgent.indexOf("EdgeInsight/");
    if (startPos == -1) return "";
    
    startPos += 12; // длина "EdgeInsight/"
    
    // Извлекаем base64 данные (до пробела или конца строки)
    int endPos = userAgent.indexOf(' ', startPos);
    if (endPos == -1) endPos = userAgent.length();
    
    String encodedData = userAgent.substring(startPos, endPos);
    
    if (encodedData.isEmpty()) return "";
    
    // Декодируем base64
    String decodedJson = decodeBase64(encodedData);
    
    if (!decodedJson.isEmpty()) {
        stats.extractedMetadata++;
        LOG_DEBUG("HeaderObfuscation", "🎭 Extracted metadata: " + decodedJson);
    }
    
    return decodedJson;
}

