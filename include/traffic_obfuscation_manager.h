#ifndef TRAFFIC_OBFUSCATION_MANAGER_H
#define TRAFFIC_OBFUSCATION_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <vector>
#include "log_manager.h"

/**
 * @brief TrafficObfuscationManager - Anti-Wireshark защита через decoy traffic
 * 
 * Генерирует ложные HTTP запросы для маскировки реального трафика ESP32
 * от packet sniffing и traffic analysis атак.
 */
class TrafficObfuscationManager {
public:
    static TrafficObfuscationManager& getInstance();
    
    bool begin();
    void end();
    void update(); // Вызывать в loop() для периодической генерации
    
    // Генерация decoy трафика
    void generateDecoyTraffic();
    bool generateRandomHttpRequests();
    void generateFakeApiCalls();
    
    // Traffic padding для реальных запросов
    void addTrafficPadding(const String& realData, String& paddedData);
    void removeTrafficPadding(const String& paddedData, String& realData);
    
    // Timing obfuscation
    void addRandomDelay();
    unsigned long getRandomInterval(unsigned long minMs, unsigned long maxMs);
    
    // Статистика
    int getDecoyRequestCount();
    void clearStatistics();

private:
    TrafficObfuscationManager();
    ~TrafficObfuscationManager();
    TrafficObfuscationManager(const TrafficObfuscationManager&) = delete;
    TrafficObfuscationManager& operator=(const TrafficObfuscationManager&) = delete;
    
    // Внутренняя логика
    bool sendDecoyHttpRequest(const String& endpoint, const String& method = "GET");
    String generateRandomUserAgent();
    String generateRandomApiPath();
    String generateRandomJsonPayload();
    
    // Состояние
    bool initialized;
    unsigned long lastDecoyTraffic;
    unsigned long decoyInterval;
    int totalDecoyRequests;
    
    // Decoy endpoints для имитации реального API
    static const char* decoyEndpoints[];
    static const char* decoyUserAgents[];
};

#endif // TRAFFIC_OBFUSCATION_MANAGER_H
