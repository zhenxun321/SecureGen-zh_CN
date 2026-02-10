#include "header_obfuscation_integration.h"
#include "log_manager.h"

String HeaderObfuscationIntegration::getClientId(AsyncWebServerRequest* request, 
                                                 HeaderObfuscationManager& obfuscationManager) {
    if (!request) return "";
    
    // Пытаемся получить деобфусцированный заголовок
    String clientId = obfuscationManager.getDeobfuscatedHeader(request, "X-Client-ID");
    
    if (clientId.isEmpty()) {
        // Fallback: проверяем оригинальный заголовок напрямую
        clientId = getHeaderWithFallback(request, "X-Req-UUID", "X-Client-ID");
    }
    
    return clientId;
}

bool HeaderObfuscationIntegration::isSecureRequest(AsyncWebServerRequest* request,
                                                   HeaderObfuscationManager& obfuscationManager) {
    if (!request) return false;
    
    // Проверяем обфусцированный заголовок
    String secureFlag = obfuscationManager.getDeobfuscatedHeader(request, "X-Secure-Request");
    
    if (!secureFlag.isEmpty()) {
        return secureFlag.equals("true") || secureFlag.equals("1");
    }
    
    // Fallback: проверяем оригинальные заголовки
    if (request->hasHeader("X-Security-Level") || request->hasHeader("X-Secure-Request")) {
        String value = getHeaderWithFallback(request, "X-Security-Level", "X-Secure-Request");
        return value.equals("true") || value.equals("1");
    }
    
    return false;
}

String HeaderObfuscationIntegration::getEmbeddedMetadata(AsyncWebServerRequest* request,
                                                        HeaderObfuscationManager& obfuscationManager) {
    if (!request) return "";
    
    return obfuscationManager.extractEmbeddedMetadata(request);
}

void HeaderObfuscationIntegration::logObfuscatedRequest(AsyncWebServerRequest* request,
                                                       const String& endpoint,
                                                       HeaderObfuscationManager& obfuscationManager) {
    if (!request) return;
    
    bool isObfuscated = obfuscationManager.isHeaderObfuscated(request);
    
    if (isObfuscated) {
        String metadata = getEmbeddedMetadata(request, obfuscationManager);
        
        LOG_DEBUG("HeaderObfuscation", 
                 "🎭 Obfuscated request to " + endpoint + 
                 (metadata.isEmpty() ? "" : " | Metadata: " + metadata));
    }
}

bool HeaderObfuscationIntegration::isLegacyRequest(AsyncWebServerRequest* request,
                                                   HeaderObfuscationManager& obfuscationManager) {
    if (!request) return true;
    
    // Legacy запрос если нет обфусцированных заголовков
    return !obfuscationManager.isHeaderObfuscated(request);
}

String HeaderObfuscationIntegration::getHeaderWithFallback(AsyncWebServerRequest* request,
                                                           const String& primary,
                                                           const String& fallback) {
    if (!request) return "";
    
    // Проверяем primary заголовок
    if (request->hasHeader(primary.c_str())) {
        return request->getHeader(primary.c_str())->value();
    }
    
    // Fallback
    if (request->hasHeader(fallback.c_str())) {
        return request->getHeader(fallback.c_str())->value();
    }
    
    return "";
}
