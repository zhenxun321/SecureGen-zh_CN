#ifndef URL_OBFUSCATION_INTEGRATION_H
#define URL_OBFUSCATION_INTEGRATION_H

#include <ESPAsyncWebServer.h>
#include "url_obfuscation_manager.h"
#include "log_manager.h"

/**
 * @brief Middleware интеграция для URL obfuscation
 * 
 * Предоставляет функции для интеграции URL obfuscation
 * в существующую архитектуру WebServerManager без поломки совместимости.
 */
class URLObfuscationIntegration {
public:
    // Middleware functions for endpoint registration
    static void registerDualEndpoint(AsyncWebServer& server, 
                                   const String& realPath,
                                   WebRequestMethodComposite method,
                                   ArRequestHandlerFunction onRequest,
                                   URLObfuscationManager& obfuscationManager);
    
    static void registerDualEndpointWithBody(AsyncWebServer& server,
                                           const String& realPath, 
                                           WebRequestMethodComposite method,
                                           ArRequestHandlerFunction onRequest,
                                           ArBodyHandlerFunction onBody,
                                           URLObfuscationManager& obfuscationManager);
    
    static void registerDualEndpointWithUpload(AsyncWebServer& server,
                                             const String& realPath,
                                             WebRequestMethodComposite method,
                                             ArRequestHandlerFunction onRequest,
                                             ArUploadHandlerFunction onUpload,
                                             ArBodyHandlerFunction onBody,
                                             URLObfuscationManager& obfuscationManager);
    
    // URL resolution API endpoints
    static void addObfuscationAPIEndpoints(AsyncWebServer& server, URLObfuscationManager& obfuscationManager);
    
    // Request processing helpers
    static String getRequestedPath(AsyncWebServerRequest* request);
    static bool isObfuscatedRequest(AsyncWebServerRequest* request, URLObfuscationManager& obfuscationManager);
    static String resolveToRealPath(AsyncWebServerRequest* request, URLObfuscationManager& obfuscationManager);
    
    // Logging and debugging
    static void logEndpointRegistration(const String& realPath, const String& obfuscatedPath);
    static void logRequestMapping(AsyncWebServerRequest* request, const String& realPath);

private:
    // Internal helper for creating request wrappers
    static ArRequestHandlerFunction createPathResolvingWrapper(ArRequestHandlerFunction originalHandler, 
                                                             URLObfuscationManager& obfuscationManager);
};

#endif // URL_OBFUSCATION_INTEGRATION_H
