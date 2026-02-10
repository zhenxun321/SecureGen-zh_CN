#ifndef HEADER_OBFUSCATION_INTEGRATION_H
#define HEADER_OBFUSCATION_INTEGRATION_H

#include <ESPAsyncWebServer.h>
#include "header_obfuscation_manager.h"
#include "log_manager.h"

/**
 * @brief Интеграционный слой для Header Obfuscation
 * 
 * Предоставляет helper функции для удобной работы с обфусцированными
 * заголовками в обработчиках эндпоинтов.
 */
class HeaderObfuscationIntegration {
public:
    /**
     * @brief Получить Client ID с поддержкой деобфускации
     * 
     * Проверяет как обфусцированный (X-Req-UUID), так и оригинальный (X-Client-ID)
     * заголовки для обратной совместимости.
     * 
     * @param request HTTP запрос
     * @param obfuscationManager Менеджер обфускации
     * @return Client ID или пустая строка
     */
    static String getClientId(AsyncWebServerRequest* request, 
                             HeaderObfuscationManager& obfuscationManager);
    
    /**
     * @brief Проверить наличие Secure Request флага с деобфускацией
     * 
     * Проверяет как обфусцированный (X-Security-Level), так и оригинальный 
     * (X-Secure-Request) заголовки.
     * 
     * @param request HTTP запрос
     * @param obfuscationManager Менеджер обфускации
     * @return true если запрос помечен как защищенный
     */
    static bool isSecureRequest(AsyncWebServerRequest* request,
                               HeaderObfuscationManager& obfuscationManager);
    
    /**
     * @brief Извлечь метаданные из User-Agent
     * 
     * Парсит встроенные в User-Agent метаданные (endpoint, method, timestamp).
     * 
     * @param request HTTP запрос
     * @param obfuscationManager Менеджер обфускации
     * @return JSON строка с метаданными или пустая строка
     */
    static String getEmbeddedMetadata(AsyncWebServerRequest* request,
                                     HeaderObfuscationManager& obfuscationManager);
    
    /**
     * @brief Логировать информацию об обфусцированном запросе
     * 
     * @param request HTTP запрос
     * @param endpoint Название эндпоинта
     * @param obfuscationManager Менеджер обфускации
     */
    static void logObfuscatedRequest(AsyncWebServerRequest* request,
                                    const String& endpoint,
                                    HeaderObfuscationManager& obfuscationManager);
    
    /**
     * @brief Проверить обратную совместимость
     * 
     * Возвращает true если запрос использует старые заголовки без обфускации.
     * 
     * @param request HTTP запрос
     * @param obfuscationManager Менеджер обфускации
     * @return true если используются оригинальные заголовки
     */
    static bool isLegacyRequest(AsyncWebServerRequest* request,
                               HeaderObfuscationManager& obfuscationManager);

private:
    // Helper для получения заголовка с fallback
    static String getHeaderWithFallback(AsyncWebServerRequest* request,
                                       const String& primary,
                                       const String& fallback);
};

#endif // HEADER_OBFUSCATION_INTEGRATION_H
