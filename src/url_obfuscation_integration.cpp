#include "url_obfuscation_integration.h"
#include <ArduinoJson.h>

void URLObfuscationIntegration::registerDualEndpoint(AsyncWebServer& server, 
                                                   const String& realPath,
                                                   WebRequestMethodComposite method,
                                                   ArRequestHandlerFunction onRequest,
                                                   URLObfuscationManager& obfuscationManager) {
    
    // Регистрируем обычный endpoint (для обратной совместимости)
    server.on(realPath.c_str(), method, onRequest);
    
    // Регистрируем обфусцированный endpoint
    String obfuscatedPath = obfuscationManager.obfuscateURL(realPath);
    if (!obfuscatedPath.isEmpty() && !obfuscatedPath.equals(realPath)) {
        server.on(obfuscatedPath.c_str(), method, createPathResolvingWrapper(onRequest, obfuscationManager));
        logEndpointRegistration(realPath, obfuscatedPath);
    }
}

void URLObfuscationIntegration::registerDualEndpointWithBody(AsyncWebServer& server,
                                                           const String& realPath, 
                                                           WebRequestMethodComposite method,
                                                           ArRequestHandlerFunction onRequest,
                                                           ArBodyHandlerFunction onBody,
                                                           URLObfuscationManager& obfuscationManager) {
    
    // Регистрируем обычный endpoint
    server.on(realPath.c_str(), method, onRequest, NULL, onBody);
    
    // Регистрируем обфусцированный endpoint
    String obfuscatedPath = obfuscationManager.obfuscateURL(realPath);
    if (!obfuscatedPath.isEmpty() && !obfuscatedPath.equals(realPath)) {
        server.on(obfuscatedPath.c_str(), method, 
                 createPathResolvingWrapper(onRequest, obfuscationManager), 
                 NULL, onBody);
        logEndpointRegistration(realPath, obfuscatedPath);
    }
}

void URLObfuscationIntegration::registerDualEndpointWithUpload(AsyncWebServer& server,
                                                             const String& realPath,
                                                             WebRequestMethodComposite method,
                                                             ArRequestHandlerFunction onRequest,
                                                             ArUploadHandlerFunction onUpload,
                                                             ArBodyHandlerFunction onBody,
                                                             URLObfuscationManager& obfuscationManager) {
    
    // Регистрируем обычный endpoint
    server.on(realPath.c_str(), method, onRequest, onUpload, onBody);
    
    // Регистрируем обфусцированный endpoint  
    String obfuscatedPath = obfuscationManager.obfuscateURL(realPath);
    if (!obfuscatedPath.isEmpty() && !obfuscatedPath.equals(realPath)) {
        server.on(obfuscatedPath.c_str(), method, 
                 createPathResolvingWrapper(onRequest, obfuscationManager), 
                 onUpload, onBody);
        logEndpointRegistration(realPath, obfuscatedPath);
    }
}

void URLObfuscationIntegration::addObfuscationAPIEndpoints(AsyncWebServer& server, URLObfuscationManager& obfuscationManager) {
    LOG_INFO("URLObfuscationAPI", "Adding URL obfuscation API endpoints");
    
    // API endpoint для получения текущих obfuscated mappings
    server.on("/api/obfuscation/mappings", HTTP_GET, [&obfuscationManager](AsyncWebServerRequest *request) {
        
        String mappingsJson = obfuscationManager.getObfuscatedMappingJSON();
        request->send(200, "application/json", mappingsJson);
    });
    
    // API endpoint для проверки статуса obfuscation
    server.on("/api/obfuscation/status", HTTP_GET, [&obfuscationManager](AsyncWebServerRequest *request) {
        
        JsonDocument doc;
        doc["enabled"] = true;
        doc["mappings_count"] = obfuscationManager.getActiveMappingsCount();
        doc["last_rotation"] = obfuscationManager.getLastRotationTime();
        doc["rotation_needed"] = obfuscationManager.needsRotation();
        
        // Добавляем информацию о критических endpoints
        JsonArray endpoints = doc["critical_endpoints"].to<JsonArray>();
        for (const String& endpoint : obfuscationManager.getAllCriticalEndpoints()) {
            JsonObject endpointObj = endpoints.add<JsonObject>();
            endpointObj["real"] = endpoint;
            endpointObj["obfuscated"] = obfuscationManager.obfuscateURL(endpoint);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // API endpoint для форсированной регенерации mappings (для debugging)
    server.on("/api/obfuscation/regenerate", HTTP_POST, [&obfuscationManager](AsyncWebServerRequest *request) {
        LOG_INFO("URLObfuscationAPI", "Manual regeneration requested");
        
        obfuscationManager.generateDailyMapping();
        
        JsonDocument doc;
        doc["status"] = "success";
        doc["message"] = "URL mappings regenerated";
        doc["new_mappings_count"] = obfuscationManager.getActiveMappingsCount();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    LOG_INFO("URLObfuscationAPI", "URL obfuscation API endpoints added");
}

String URLObfuscationIntegration::getRequestedPath(AsyncWebServerRequest* request) {
    if (!request) return "";
    return request->url();
}

bool URLObfuscationIntegration::isObfuscatedRequest(AsyncWebServerRequest* request, URLObfuscationManager& obfuscationManager) {
    if (!request) return false;
    
    String requestedPath = getRequestedPath(request);
    return obfuscationManager.isObfuscatedPath(requestedPath);
}

String URLObfuscationIntegration::resolveToRealPath(AsyncWebServerRequest* request, URLObfuscationManager& obfuscationManager) {
    if (!request) return "";
    
    String requestedPath = getRequestedPath(request);
    
    // Если это обфусцированный path, resolve to real
    if (obfuscationManager.isObfuscatedPath(requestedPath)) {
        String realPath = obfuscationManager.getRealPath(requestedPath);
        logRequestMapping(request, realPath);
        return realPath;
    }
    
    // Иначе возвращаем как есть
    return requestedPath;
}

void URLObfuscationIntegration::logEndpointRegistration(const String& realPath, const String& obfuscatedPath) {
    // Логирование отключено для снижения нагрузки
}

void URLObfuscationIntegration::logRequestMapping(AsyncWebServerRequest* request, const String& realPath) {
    // Логирование отключено для производительности
}

ArRequestHandlerFunction URLObfuscationIntegration::createPathResolvingWrapper(ArRequestHandlerFunction originalHandler, 
                                                                             URLObfuscationManager& obfuscationManager) {
    
    return [originalHandler, &obfuscationManager](AsyncWebServerRequest *request) {
        // Log the obfuscated request for debugging
        String requestedPath = getRequestedPath(request);
        String realPath = resolveToRealPath(request, obfuscationManager);
        
        // Вызываем оригинальный handler
        originalHandler(request);
    };
}
