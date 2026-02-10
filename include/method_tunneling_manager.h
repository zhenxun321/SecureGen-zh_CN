#ifndef METHOD_TUNNELING_MANAGER_H
#define METHOD_TUNNELING_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <map>
#include "crypto_manager.h"
#include "log_manager.h"

#ifdef SECURE_LAYER_ENABLED
#include "secure_layer_manager.h"
#endif

/**
 * @brief MethodTunnelingManager - Скрытие HTTP методов от Wireshark через POST туннелирование
 * 
 * Все HTTP методы (GET, POST, PUT, DELETE) туннелируются через POST запросы
 * с зашифрованным заголовком X-Real-Method для защиты от traffic analysis.
 * 
 * Интегрируется с SecureLayerManager для двойного шифрования:
 * - Layer 1: Method Tunneling (скрывает HTTP метод)
 * - Layer 2: SecureLayerManager (шифрует payload)
 */
class MethodTunnelingManager {
public:
    static MethodTunnelingManager& getInstance();
    
    // Инициализация и lifecycle
    bool begin();
    void end();
    
    // Серверная сторона - дешифровка методов
    String decryptMethodHeader(const String& encryptedMethod, const String& clientId = "");
    bool isTunneledRequest(AsyncWebServerRequest* request);
    String extractRealMethod(AsyncWebServerRequest* request);
    
    // Клиентская сторона - шифровка методов  
    String encryptMethodHeader(const String& method, const String& clientId = "");
    
    // Интеграция с AsyncWebServer
    void registerTunneledEndpoint(AsyncWebServer& server, const String& path, 
                                 ArRequestHandlerFunction onRequest);
    void registerTunneledEndpointWithBody(AsyncWebServer& server, const String& path,
                                         ArRequestHandlerFunction onRequest,
                                         ArBodyHandlerFunction onBody);
    
    // Middleware функции
    bool shouldProcessTunneling(const String& endpoint);
    String wrapTunneledRequest(AsyncWebServerRequest* request, const String& originalBody);
    
    // Статистика и отладка
    int getTunneledRequestCount();
    void clearStatistics();
    
private:
    MethodTunnelingManager();
    ~MethodTunnelingManager();
    MethodTunnelingManager(const MethodTunnelingManager&) = delete;
    MethodTunnelingManager& operator=(const MethodTunnelingManager&) = delete;
    
    // Криптография для заголовков
    bool encryptHeaderData(const String& plaintext, const String& clientId, String& encrypted);
    bool decryptHeaderData(const String& encrypted, const String& clientId, String& plaintext);
    
    // Внутренняя логика
    String generateMethodEncryptionKey(const String& clientId);
    bool isValidHttpMethod(const String& method);
    
    // Fallback шифрование (простое XOR) для случаев без SecureLayerManager
    String xorEncryptMethod(const String& method, const String& key);
    String xorDecryptMethod(const String& encrypted, const String& key);
    
    // Состояние
    bool initialized;
    std::map<String, int> tunneledEndpoints; // Статистика по endpoints
    unsigned long totalTunneledRequests;
    
    // Список endpoints для туннелирования
    static const char* tunnelingEnabledEndpoints[];
    
#ifdef SECURE_LAYER_ENABLED
    SecureLayerManager& secureLayer = SecureLayerManager::getInstance();
#endif
    CryptoManager& crypto = CryptoManager::getInstance();
};

#endif // METHOD_TUNNELING_MANAGER_H
