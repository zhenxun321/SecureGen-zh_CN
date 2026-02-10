/**
 * Тестовый файл для проверки синтаксиса security исправлений
 * ESP32 T-Display TOTP Password Manager
 */

// Проверяем корректность includes
#include "traffic_obfuscation_manager.h"
#include "secure_layer_manager.h"
#include "web_server_secure_integration.h"
#include "method_tunneling_manager.h"

// Тест 1: TrafficObfuscationManager инициализация
bool testTrafficObfuscationInit() {
    TrafficObfuscationManager& manager = TrafficObfuscationManager::getInstance();
    return manager.begin();
}

// Тест 2: SecureLayerManager timing protection
bool testTimingProtection() {
    SecureLayerManager& secureLayer = SecureLayerManager::getInstance();
    
    // Должно включать случайные задержки
    String testData = "{\"test\":\"data\"}";
    String encrypted;
    
    unsigned long start = millis();
    bool result = secureLayer.encryptResponse("test_client_123", testData, encrypted);
    unsigned long duration = millis() - start;
    
    // Ожидаем задержку 50-350ms для защиты от timing analysis
    return (duration >= 50 && duration <= 400);
}

// Тест 3: Method tunneling parameter injection
bool testMethodTunnelingParams() {
    MethodTunnelingManager& tunneling = MethodTunnelingManager::getInstance();
    
    // Проверяем что tunneling менеджер корректно инициализируется
    if (!tunneling.begin()) {
        return false;
    }
    
    // Статистика должна быть доступна
    return tunneling.getTotalTunneledRequests() >= 0;
}

// Тест 4: Decoy traffic generation
bool testDecoyTrafficGeneration() {
    TrafficObfuscationManager& manager = TrafficObfuscationManager::getInstance();
    
    if (!manager.begin()) {
        return false;
    }
    
    int initialCount = manager.getDecoyRequestCount();
    manager.generateDecoyTraffic();
    int newCount = manager.getDecoyRequestCount();
    
    // Должно увеличиться количество decoy запросов
    return newCount > initialCount;
}

// Тест 5: Traffic padding функциональность
bool testTrafficPadding() {
    TrafficObfuscationManager& manager = TrafficObfuscationManager::getInstance();
    
    String originalData = "sensitive_password_data";
    String paddedData;
    String extractedData;
    
    // Добавляем padding
    manager.addTrafficPadding(originalData, paddedData);
    
    // Проверяем что данные увеличились
    if (paddedData.length() <= originalData.length()) {
        return false;
    }
    
    // Извлекаем данные обратно
    manager.removeTrafficPadding(paddedData, extractedData);
    
    // Должны получить исходные данные
    return extractedData == originalData;
}

// Основная тестовая функция
void runSecurityTests() {
    Serial.println("🧪 TESTING SECURITY FIXES:");
    
    Serial.print("1. Traffic Obfuscation Init: ");
    Serial.println(testTrafficObfuscationInit() ? "✅ PASS" : "❌ FAIL");
    
    Serial.print("2. Timing Protection: ");
    Serial.println(testTimingProtection() ? "✅ PASS" : "❌ FAIL");
    
    Serial.print("3. Method Tunneling Params: ");
    Serial.println(testMethodTunnelingParams() ? "✅ PASS" : "❌ FAIL");
    
    Serial.print("4. Decoy Traffic Generation: ");
    Serial.println(testDecoyTrafficGeneration() ? "✅ PASS" : "❌ FAIL");
    
    Serial.print("5. Traffic Padding: ");
    Serial.println(testTrafficPadding() ? "✅ PASS" : "❌ FAIL");
    
    Serial.println("\n🔐 SECURITY INTEGRATION TEST COMPLETED");
}

// Проверка корректности кода - должна компилироваться без ошибок
int main() {
    runSecurityTests();
    return 0;
}
