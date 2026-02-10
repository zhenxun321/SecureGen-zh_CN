#ifndef HEADER_OBFUSCATION_MANAGER_H
#define HEADER_OBFUSCATION_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include <vector>
#include "log_manager.h"

/**
 * @brief Менеджер обфускации HTTP заголовков
 * 
 * Обеспечивает деобфускацию входящих заголовков от клиента
 * и поддержку обратной совместимости с необфусцированными запросами.
 * 
 * Поддерживаемые техники:
 * - Header Mapping: переименование заголовков (X-Client-ID → X-Req-UUID)
 * - Fake Headers Detection: игнорирование ложных заголовков
 * - User-Agent Payload Extraction: извлечение встроенных метаданных
 */
class HeaderObfuscationManager {
public:
    static HeaderObfuscationManager& getInstance();
    
    // Lifecycle
    bool begin();
    
    // Request processing
    bool isHeaderObfuscated(AsyncWebServerRequest* request);
    String getDeobfuscatedHeader(AsyncWebServerRequest* request, const String& originalHeaderName);
    String extractEmbeddedMetadata(AsyncWebServerRequest* request);
    
    // Header mapping configuration
    void setHeaderMapping(const String& obfuscated, const String& real);
    void registerFakeHeader(const String& headerName);
    
    // Statistics
    struct ObfuscationStats {
        uint32_t totalRequests;
        uint32_t obfuscatedRequests;
        uint32_t extractedMetadata;
        unsigned long lastObfuscatedRequest;
    };
    ObfuscationStats getStats() const { return stats; }
    
    // Enable/Disable
    void setEnabled(bool enabled) { this->enabled = enabled; }
    bool isEnabled() const { return enabled; }

private:
    HeaderObfuscationManager();
    ~HeaderObfuscationManager();
    HeaderObfuscationManager(const HeaderObfuscationManager&) = delete;
    HeaderObfuscationManager& operator=(const HeaderObfuscationManager&) = delete;
    
    // Header mapping storage
    // obfuscatedToReal: "X-Req-UUID" → "X-Client-ID"
    std::map<String, String> obfuscatedToReal;
    
    // Fake headers to ignore
    std::vector<String> fakeHeaders;
    
    // Statistics
    ObfuscationStats stats;
    
    // Configuration
    bool initialized;
    bool enabled;
    
    // Helper functions
    String decodeBase64(const String& encoded);
    bool hasObfuscatedHeaders(AsyncWebServerRequest* request);
    void initializeDefaultMappings();
};

#endif // HEADER_OBFUSCATION_MANAGER_H
