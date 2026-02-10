#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "wifi_manager.h"
#include "log_manager.h"
#include "crypto_manager.h"  // Для шифрования WiFi credentials
#include "config.h"  // Для WIFI_CONFIG_FILE

WifiManager::WifiManager(DisplayManager& display, ConfigManager& configManager) 
    : _display(display), _configManager(configManager) {}

bool WifiManager::loadCredentials(String& ssid, String& password) {
    LOG_DEBUG("WifiManager", "Loading WiFi credentials");
    
    // 🔒 Проверяем зашифрованный файл
    if (LittleFS.exists(WIFI_CONFIG_FILE)) {
        LOG_DEBUG("WifiManager", "Loading encrypted WiFi config");
        File file = LittleFS.open(WIFI_CONFIG_FILE, "r");
        if (!file) {
            LOG_ERROR("WifiManager", "Failed to open encrypted WiFi config file");
            return false;
        }

        String encrypted_base64 = file.readString();
        file.close();

        if (encrypted_base64.length() == 0) {
            LOG_WARNING("WifiManager", "Encrypted WiFi config file is empty");
            return false;
        }

        // Дешифруем данные
        String json_string = CryptoManager::getInstance().decrypt(encrypted_base64);
        if (json_string.length() == 0) {
            LOG_ERROR("WifiManager", "Failed to decrypt WiFi config");
            return false;
        }
        
        // Парсим JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json_string);
        if (error) {
            LOG_ERROR("WifiManager", "JSON parsing failed for WiFi config: " + String(error.c_str()));
            return false;
        }

        ssid = doc["ssid"].as<String>();
        password = doc["password"].as<String>();
        
        if (ssid.length() == 0) {
            LOG_WARNING("WifiManager", "Empty SSID in WiFi config");
            return false;
        }
        
        LOG_INFO("WifiManager", "WiFi credentials loaded (encrypted) for SSID: " + ssid);
        return true;
    }
    
    // 🔄 Миграция: проверяем старый plain text файл
    if (LittleFS.exists(WIFI_CONFIG_FILE_LEGACY)) {
        LOG_WARNING("WifiManager", "Found legacy plain text WiFi config - migrating to encrypted");
        
        File configFile = LittleFS.open(WIFI_CONFIG_FILE_LEGACY, "r");
        if (!configFile) {
            LOG_ERROR("WifiManager", "Failed to open legacy WiFi config file");
            return false;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, configFile);
        configFile.close();
        
        if (error) {
            LOG_ERROR("WifiManager", "Failed to parse legacy WiFi config: " + String(error.c_str()));
            return false;
        }

        ssid = doc["ssid"].as<String>();
        password = doc["password"].as<String>();
        
        if (ssid.length() == 0) {
            LOG_WARNING("WifiManager", "Empty SSID in legacy WiFi config");
            return false;
        }
        
        // 🔒 Сохраняем в зашифрованном виде
        if (saveCredentials(ssid, password)) {
            LOG_INFO("WifiManager", "Successfully migrated WiFi credentials to encrypted file");
            
            // Удаляем старый plain text файл
            if (LittleFS.remove(WIFI_CONFIG_FILE_LEGACY)) {
                LOG_INFO("WifiManager", "Removed legacy plain text WiFi config");
            } else {
                LOG_WARNING("WifiManager", "Failed to remove legacy plain text file");
            }
        } else {
            LOG_ERROR("WifiManager", "Failed to migrate WiFi credentials");
        }
        
        LOG_INFO("WifiManager", "WiFi credentials loaded (migrated) for SSID: " + ssid);
        return true;
    }
    
    LOG_INFO("WifiManager", "No WiFi config file found (neither encrypted nor legacy)");
    return false;
}

bool WifiManager::connect() {
    LOG_INFO("WifiManager", "Starting WiFi connection");
    String ssid, password;
    
    _display.showMessage("Loading WiFi config...", 10, 50);
    delay(500);

    if (!loadCredentials(ssid, password)) {
        LOG_ERROR("WifiManager", "Failed to load WiFi credentials");
        _display.showMessage("No WiFi config found.", 10, 50, true);
        delay(1500);
        return false;
    }

    _display.showMessage("Config loaded.", 10, 50);
    _display.showMessage("Connecting to:", 10, 70);
    _display.showMessage(ssid, 10, 90, false, 2);

    // --- Улучшенная логика подключения ---
    WiFi.mode(WIFI_STA); // Явно устанавливаем режим клиента
    WiFi.disconnect(true); // Очищаем предыдущую сессию
    delay(100);

    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    // Увеличиваем время ожидания до 20 секунд (40 * 500 мс)
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        _ipAddress = WiFi.localIP().toString();
        LOG_INFO("WifiManager", "WiFi connected successfully. IP: " + _ipAddress);
        
        // Start mDNS service
        startMdnsService();
        
        return true; // Успех!
    }
    
    // Если не удалось, показываем ошибку и возвращаем false
    LOG_ERROR("WifiManager", "WiFi connection failed after " + String(attempts) + " attempts");
    _display.init();
    _display.showMessage("Connection Failed!", 10, 50, true);
    _display.showMessage("Check credentials.", 10, 70);
    delay(2500);
    WiFi.disconnect();
    return false; // Неудача
}

void WifiManager::startConfigPortal() {
    const char* ap_ssid = "ESP32-TOTP-Setup";
    LOG_INFO("WifiManager", "Starting WiFi config portal: " + String(ap_ssid));
    _display.init();
    _display.showMessage("WiFi Setup Mode", 10, 10, false, 2);
    _display.showMessage("1. Connect to WiFi:", 10, 40);
    _display.showMessage(ap_ssid, 15, 60, false, 2);
    _display.showMessage("2. Go to 192.168.4.1", 10, 90);

    bool apStarted = WiFi.softAP(ap_ssid);
    if (apStarted) {
        LOG_INFO("WifiManager", "Access point started successfully");
    } else {
        LOG_ERROR("WifiManager", "Failed to start access point");
    }
}

String WifiManager::getIP() {
    return _ipAddress;
}

bool WifiManager::connectSilent() {
    LOG_INFO("WifiManager", "Starting silent WiFi connection");
    String ssid, password;
    
    if (!loadCredentials(ssid, password)) {
        LOG_ERROR("WifiManager", "Failed to load WiFi credentials for silent connection");
        return false;
    }

    // --- Тихое подключение без сообщений на экране ---
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);

    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        _ipAddress = WiFi.localIP().toString();
        LOG_INFO("WifiManager", "WiFi silently connected. IP: " + _ipAddress);
        
        // КРИТИЧНО: Запускаем mDNS сервис после тихого подключения
        startMdnsService();
        
        return true;
    }
    
    LOG_ERROR("WifiManager", "Silent WiFi connection failed after " + String(attempts) + " attempts");
    WiFi.disconnect();
    return false;
}

void WifiManager::disconnect() {
    LOG_INFO("WifiManager", "Disconnecting WiFi");
    
    // КРИТИЧНО: Останавливаем mDNS перед отключением WiFi
    MDNS.end();
    LOG_DEBUG("WifiManager", "mDNS service stopped");
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    LOG_INFO("WifiManager", "WiFi disconnected");
}

void WifiManager::updateMdnsHostname() {
    String hostname = _configManager.loadMdnsHostname();
    MDNS.setInstanceName(hostname);
    LOG_INFO("WifiManager", "mDNS hostname updated to: " + hostname);
}

void WifiManager::startMdnsService() {
    String hostname = _configManager.loadMdnsHostname();
    
    // КРИТИЧНО: Сначала останавливаем предыдущий mDNS если он был запущен
    MDNS.end();
    
    if (MDNS.begin(hostname.c_str())) {
        LOG_INFO("WifiManager", "mDNS responder started. Access via: http://" + hostname + ".local");
        if (!MDNS.addService("http", "tcp", 80)) {
            LOG_ERROR("WifiManager", "Failed to add HTTP service to mDNS");
        }
    } else {
        LOG_ERROR("WifiManager", "Failed to start mDNS responder");
    }
}

// 🔒 Сохранение зашифрованных WiFi credentials
bool WifiManager::saveCredentials(const String& ssid, const String& password) {
    LOG_DEBUG("WifiManager", "Saving encrypted WiFi credentials");
    
    // Создаём JSON с credentials
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["password"] = password;
    
    String json_string;
    size_t jsonSize = serializeJson(doc, json_string);
    if (jsonSize == 0) {
        LOG_ERROR("WifiManager", "Failed to serialize WiFi credentials to JSON");
        return false;
    }

    // Шифруем данные
    String encrypted_base64 = CryptoManager::getInstance().encrypt(json_string);
    if (encrypted_base64.length() == 0) {
        LOG_ERROR("WifiManager", "Failed to encrypt WiFi credentials");
        return false;
    }

    // Сохраняем в файл
    File file = LittleFS.open(WIFI_CONFIG_FILE, "w");
    if (!file) {
        LOG_ERROR("WifiManager", "Failed to open WiFi config file for writing");
        return false;
    }
    
    size_t bytesWritten = file.print(encrypted_base64);
    file.close();
    
    if (bytesWritten > 0) {
        LOG_INFO("WifiManager", "WiFi credentials saved successfully (encrypted) for SSID: " + ssid);
        return true;
    } else {
        LOG_ERROR("WifiManager", "Failed to write encrypted WiFi credentials");
        return false;
    }
}