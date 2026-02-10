#ifndef WEB_SERVER_SECURE_INTEGRATION_H
#define WEB_SERVER_SECURE_INTEGRATION_H

#include "web_server.h"
#include "secure_layer_manager.h"
#include "url_obfuscation_manager.h"
#include <ESPAsyncWebServer.h>

/**
 * @brief Интеграционные методы для внедрения SecureLayerManager в WebServerManager
 * 
 * Эти методы должны быть добавлены в WebServerManager для поддержки HTTPS-like шифрования
 */

class WebServerSecureIntegration {
public:
    /**
     * @brief Добавляет secure endpoints к существующему web server
     */
    static void addSecureEndpoints(AsyncWebServer& server, SecureLayerManager& secureLayer, URLObfuscationManager& urlObfuscation);
    
    /**
     * @brief Middleware для автоматического шифрования/дешифрования
     */
    static bool processSecureRequest(AsyncWebServerRequest* request, SecureLayerManager& secureLayer, String& processedBody);
    
    /**
     * @brief Обертка для безопасных ответов
     * ⚡ IRAM_ATTR - hot-path функция, вызывается при каждом зашифрованном ответе
     */
    static IRAM_ATTR void sendSecureResponse(AsyncWebServerRequest* request, int code, const String& contentType, const String& content, SecureLayerManager& secureLayer);
    
    /**
     * @brief Получение client ID из запроса
     */
    static String getClientId(AsyncWebServerRequest* request);
};

#endif // WEB_SERVER_SECURE_INTEGRATION_H
