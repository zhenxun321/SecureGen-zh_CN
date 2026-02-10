#ifndef URL_OBFUSCATION_MANAGER_H
#define URL_OBFUSCATION_MANAGER_H

#include <Arduino.h>
#include <map>
#include <vector>
#include "crypto_manager.h"
#include "log_manager.h"

/**
 * @brief Менеджер обфускации URL для защиты от анализа трафика
 * 
 * Обеспечивает динамическое сопоставление реальных URL с обфусцированными
 * для усложнения перехвата и анализа API эндпоинтов.
 */
class URLObfuscationManager {
public:
    static URLObfuscationManager& getInstance();
    
    // Инициализация и lifecycle
    bool begin();
    void update(); // Проверка необходимости ротации
    
    // URL mapping operations
    String obfuscateURL(const String& realPath);
    String deobfuscateURL(const String& obfuscatedPath);
    bool isObfuscatedPath(const String& path);
    String getRealPath(const String& obfuscatedPath);
    
    // Daily rotation management
    void generateDailyMapping();
    bool needsRotation();
    unsigned long getLastRotationTime() const { return lastRotationTime; }
    
    // URL registration for critical endpoints
    bool registerCriticalEndpoint(const String& realPath, const String& description = "");
    std::vector<String> getAllCriticalEndpoints();
    
    // Client API support
    String getObfuscatedMappingJSON(); // Для передачи клиенту
    int getActiveMappingsCount() const { return realToObfuscated.size(); }

private:
    URLObfuscationManager();
    ~URLObfuscationManager();
    URLObfuscationManager(const URLObfuscationManager&) = delete;
    URLObfuscationManager& operator=(const URLObfuscationManager&) = delete;
    
    // Internal mapping storage
    std::map<String, String> realToObfuscated;
    std::map<String, String> obfuscatedToReal;
    std::vector<String> criticalEndpoints;
    
    // Boot-based rotation management
    uint32_t bootCounter;                                        // Счетчик перезагрузок
    static const uint32_t ROTATION_THRESHOLD = 30;              // Ротация каждые 30 перезагрузок
    unsigned long lastRotationTime;                              // Deprecated: используется только для логов
    
    // URL generation
    String generateObfuscatedPath(const String& realPath, uint32_t seed);
    String generateSecureHash(const String& input, uint32_t seed);
    uint32_t getCurrentSeed();                                   // Seed на основе bootCounter
    
    // Boot counter management
    uint32_t loadBootCounter();                                  // Загрузка счетчика из flash
    void saveBootCounter(uint32_t counter);                      // Сохранение счетчика в flash
    void incrementBootCounter();                                 // Увеличение при старте
    
    // Persistent storage
    bool loadMappingsFromStorage(uint32_t seed);
    bool saveMappingsToStorage(uint32_t seed);
    void cleanupOldMappings(uint32_t currentSeed);
    
    // Configuration
    bool initialized;
    static const size_t MAX_MAPPINGS = 50; // Ограничение памяти
    static const size_t OBFUSCATED_PATH_LENGTH = 12; // Длина обфусцированного пути
};

#endif // URL_OBFUSCATION_MANAGER_H
