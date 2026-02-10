#include "traffic_obfuscation_manager.h"
#include <esp_system.h>

// Decoy API endpoints для имитации реального трафика
const char* TrafficObfuscationManager::decoyEndpoints[] = {
    "/api/status",
    "/api/health", 
    "/api/version",
    "/api/metrics",
    "/api/ping",
    "/api/config/general",
    "/api/system/info",
    "/favicon.ico",
    "/robots.txt",
    nullptr
};

// Fake User-Agents для decoy requests
const char* TrafficObfuscationManager::decoyUserAgents[] = {
    "Mozilla/5.0 (compatible; ESP32-Monitor)",
    "ESP32-Status-Checker/1.0", 
    "IoT-Device-Health/2.1",
    "ESP32-Metrics-Collector",
    "System-Monitor-Bot",
    nullptr
};

TrafficObfuscationManager& TrafficObfuscationManager::getInstance() {
    static TrafficObfuscationManager instance;
    return instance;
}

TrafficObfuscationManager::TrafficObfuscationManager() 
    : initialized(false), lastDecoyTraffic(0), decoyInterval(30000), totalDecoyRequests(0) {
}

TrafficObfuscationManager::~TrafficObfuscationManager() {
    end();
}

bool TrafficObfuscationManager::begin() {
    if (initialized) return true;
    
    LOG_INFO("TrafficObfuscation", "🎭 Initializing Traffic Obfuscation Manager...");
    
    // Случайный интервал между decoy запросами (20-60 секунд)
    decoyInterval = 20000 + (esp_random() % 40000);
    lastDecoyTraffic = millis();
    totalDecoyRequests = 0;
    
    initialized = true;
    LOG_INFO("TrafficObfuscation", "🎭 Traffic Obfuscation initialized [interval:" + String(decoyInterval/1000) + "s]");
    return true;
}

void TrafficObfuscationManager::end() {
    if (!initialized) return;
    
    LOG_INFO("TrafficObfuscation", "🎭 Shutting down Traffic Obfuscation Manager");
    initialized = false;
}

void TrafficObfuscationManager::update() {
    if (!initialized) return;
    
    unsigned long now = millis();
    
    // Генерируем decoy трафик с случайными интервалами
    if (now - lastDecoyTraffic >= decoyInterval) {
        generateDecoyTraffic();
        lastDecoyTraffic = now;
        
        // Меняем интервал для следующего раза (непредсказуемость)
        decoyInterval = 15000 + (esp_random() % 45000); // 15-60 секунд
    }
}

void TrafficObfuscationManager::generateDecoyTraffic() {
    if (!WiFi.isConnected()) return;
    
    // Генерируем 2-4 случайных decoy запроса
    int requestCount = 2 + (esp_random() % 3);
    
    LOG_DEBUG("TrafficObfuscation", "🎭 Generating " + String(requestCount) + " decoy requests...");
    
    int successCount = 0;
    for (int i = 0; i < requestCount; i++) {
        if (generateRandomHttpRequests()) {
            successCount++;
        }
        delay(200 + (esp_random() % 800)); // Случайная задержка между запросами
    }
    
    LOG_INFO("TrafficObfuscation", "🎭 Sent " + String(successCount) + "/" + String(requestCount) + " decoy requests [Total: " + String(totalDecoyRequests) + "]");
}

bool TrafficObfuscationManager::generateRandomHttpRequests() {
    // Выбираем случайный decoy endpoint
    int endpointIndex = esp_random() % 9; // Количество endpoints в массиве
    String endpoint = String(decoyEndpoints[endpointIndex]);
    
    // Случайно выбираем HTTP метод (больше GET для реализма)
    String method = "GET";
    if ((esp_random() % 100) < 20) { // 20% вероятность POST
        method = "POST";
    }
    
    return sendDecoyHttpRequest(endpoint, method);
}

bool TrafficObfuscationManager::sendDecoyHttpRequest(const String& endpoint, const String& method) {
    // 🛡️ КРИТИЧНО: Проверяем WiFi ДО создания UDP!
    if (!WiFi.isConnected() || WiFi.status() != WL_CONNECTED) {
        return false; // WiFi не готов - не отправляем
    }
    
    // 🛡️ ОПТИМИЗИРОВАННЫЙ: статический буфер вместо String → нет heap overflow
    static char packet[256]; // Статический буфер - не в heap!
    
    WiFiUDP udp;
    
    // 🎯 Генерируем правдоподобный IP адрес
    IPAddress fakeIP;
    
    uint32_t choice = esp_random() % 100;
    
    if (choice < 30) {
        // 30% - Публичные DNS сервера (Google, Cloudflare, Quad9)
        const IPAddress publicDNS[] = {
            IPAddress(8, 8, 8, 8),        // Google DNS
            IPAddress(8, 8, 4, 4),        // Google DNS Secondary
            IPAddress(1, 1, 1, 1),        // Cloudflare DNS
            IPAddress(1, 0, 0, 1),        // Cloudflare DNS Secondary
            IPAddress(9, 9, 9, 9)         // Quad9 DNS
        };
        fakeIP = publicDNS[esp_random() % 5];
    } else if (choice < 60) {
        // 30% - Локальная подсеть (192.168.0.x - роутер и другие устройства)
        fakeIP = IPAddress(192, 168, 0, 1 + (esp_random() % 254));
    } else {
        // 40% - Другие локальные подсети (192.168.1-255.x)
        fakeIP = IPAddress(192, 168, 1 + (esp_random() % 255), 1 + (esp_random() % 254));
    }
    
    // Формируем пакет без String конкатенации
    int agentIndex = esp_random() % 5;
    
    int len = snprintf(packet, sizeof(packet),
        "%s %s HTTP/1.1\r\n"
        "Host: %d.%d.%d.%d\r\n"
        "User-Agent: %s\r\n"
        "Accept: application/json\r\n"
        "Connection: close\r\n"
        "\r\n",
        method.c_str(),
        endpoint.c_str(),
        fakeIP[0], fakeIP[1], fakeIP[2], fakeIP[3],
        decoyUserAgents[agentIndex]
    );
    
    if (len < 0 || len >= (int)sizeof(packet)) {
        return false; // Buffer overflow protection
    }
    
    // Отправляем UDP пакет
    if (udp.beginPacket(fakeIP, 80)) {
        udp.write((const uint8_t*)packet, len);
        bool sent = udp.endPacket();
        
        if (sent) {
            totalDecoyRequests++;
            LOG_DEBUG("TrafficObfuscation", "📤 Decoy " + method + " " + endpoint + " → " + 
                     String(fakeIP[0]) + "." + String(fakeIP[1]) + "." + String(fakeIP[2]) + "." + String(fakeIP[3]) + ":80");
            return true;
        }
    }
    
    LOG_WARNING("TrafficObfuscation", "⚠️ Failed to send decoy request");
    return false;
}

String TrafficObfuscationManager::generateRandomUserAgent() {
    int agentIndex = esp_random() % 5; // Количество user agents в массиве
    return String(decoyUserAgents[agentIndex]);
}

String TrafficObfuscationManager::generateRandomJsonPayload() {
    // Генерируем случайный JSON payload для POST decoy запросов
    String payload = "{";
    payload += "\"timestamp\":" + String(millis()) + ",";
    payload += "\"random_id\":\"" + String(esp_random(), HEX) + "\",";
    payload += "\"status\":\"checking\",";
    payload += "\"value\":" + String(esp_random() % 1000);
    payload += "}";
    
    return payload;
}

void TrafficObfuscationManager::addTrafficPadding(const String& realData, String& paddedData) {
    // Добавляем случайные данные до и после реального payload
    String frontPadding = "";
    String backPadding = "";
    
    // Генерируем случайное количество padding (50-200 байт)
    int frontSize = 50 + (esp_random() % 150);
    int backSize = 50 + (esp_random() % 150);
    
    LOG_DEBUG("TrafficObfuscation", "📦 Adding padding: front=" + String(frontSize) + "b, back=" + String(backSize) + "b");
    
    for (int i = 0; i < frontSize; i++) {
        frontPadding += char('A' + (esp_random() % 26));
    }
    
    for (int i = 0; i < backSize; i++) {
        backPadding += char('a' + (esp_random() % 26));
    }
    
    // Формат: [FRONT_PADDING]||REAL_DATA||[BACK_PADDING]
    paddedData = frontPadding + "||" + realData + "||" + backPadding;
    
    LOG_DEBUG("TrafficObfuscation", "✅ Padded: " + String(realData.length()) + "b → " + String(paddedData.length()) + "b (" + 
             String((paddedData.length() * 100) / realData.length()) + "% overhead)");
}

void TrafficObfuscationManager::removeTrafficPadding(const String& paddedData, String& realData) {
    // Извлекаем реальные данные между разделителями ||
    int startMarker = paddedData.indexOf("||");
    if (startMarker == -1) {
        realData = paddedData; // Нет padding
        LOG_DEBUG("TrafficObfuscation", "📦 No padding markers found");
        return;
    }
    
    int endMarker = paddedData.indexOf("||", startMarker + 2);
    if (endMarker == -1) {
        realData = paddedData; // Некорректный формат
        LOG_WARNING("TrafficObfuscation", "⚠️ Invalid padding format");
        return;
    }
    
    realData = paddedData.substring(startMarker + 2, endMarker);
    LOG_DEBUG("TrafficObfuscation", "✅ Removed padding: " + String(paddedData.length()) + "b → " + String(realData.length()) + "b");
}

void TrafficObfuscationManager::addRandomDelay() {
    // Случайная задержка 100-500ms для защиты от timing analysis
    unsigned long delay_ms = 100 + (esp_random() % 400);
    delay(delay_ms);
}

unsigned long TrafficObfuscationManager::getRandomInterval(unsigned long minMs, unsigned long maxMs) {
    if (maxMs <= minMs) return minMs;
    return minMs + (esp_random() % (maxMs - minMs));
}

int TrafficObfuscationManager::getDecoyRequestCount() {
    return totalDecoyRequests;
}

void TrafficObfuscationManager::clearStatistics() {
    totalDecoyRequests = 0;
    LOG_INFO("TrafficObfuscation", "🎭 Statistics cleared");
}
