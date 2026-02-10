#include "key_manager.h"
#include "config.h"
#include "crypto_manager.h"
#include "log_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <algorithm>
#include <map>

KeyManager::KeyManager() {}

bool KeyManager::begin() {
    LOG_INFO("KeyManager", "Initializing...");
    bool success = loadKeys();
    if (success) {
        LOG_INFO("KeyManager", "Initialized successfully");
    } else {
        LOG_ERROR("KeyManager", "Failed to initialize");
    }
    return success;
}

bool KeyManager::addKey(const String& name, const String& secret) {
    if (name.isEmpty() || secret.isEmpty()) {
        LOG_WARNING("KeyManager", "Cannot add key with empty name or secret");
        return false;
    }
    for (const auto& key : keys) {
        if (key.name == name) {
            LOG_WARNING("KeyManager", "Key already exists: " + name);
            return false;
        }
    }
    // Найти максимальный порядок для нового ключа
    int maxOrder = 0;
    for (const auto& key : keys) {
        if (key.order > maxOrder) maxOrder = key.order;
    }
    TOTPKey newKey;
    newKey.name = name;
    newKey.secret = secret; 
    newKey.order = maxOrder + 1;
    keys.push_back(newKey);
    LOG_INFO("KeyManager", "Added TOTP key: " + name);
    bool success = saveKeys();
    if (!success) {
        LOG_ERROR("KeyManager", "Failed to save keys after adding: " + name);
    }
    return success;
}

bool KeyManager::updateKey(int index, const String& name, const String& secret) {
    if (index < 0 || index >= keys.size()) {
        LOG_WARNING("KeyManager", "Invalid key index for update: " + String(index));
        return false;
    }
    if (name.isEmpty() || secret.isEmpty()) {
        LOG_WARNING("KeyManager", "Cannot update key with empty name or secret");
        return false;
    }
    keys[index].name = name;
    keys[index].secret = secret;
    // порядок остается прежний
    LOG_INFO("KeyManager", "Updated TOTP key at index " + String(index) + " to: " + name);
    bool success = saveKeys();
    if (!success) {
        LOG_ERROR("KeyManager", "Failed to save keys after update");
    }
    return success;
}

bool KeyManager::removeKey(int index) {
    if (index < 0 || index >= keys.size()) {
        LOG_WARNING("KeyManager", "Invalid key index for removal: " + String(index));
        return false;
    }
    String removedName = keys[index].name;
    keys.erase(keys.begin() + index);
    LOG_INFO("KeyManager", "Removed TOTP key: " + removedName);
    bool success = saveKeys();
    if (!success) {
        LOG_ERROR("KeyManager", "Failed to save keys after removal");
    }
    return success;
}

std::vector<TOTPKey> KeyManager::getAllKeys() {
    // Сортируем ключи по порядку перед возвратом
    std::sort(keys.begin(), keys.end(), [](const TOTPKey& a, const TOTPKey& b) {
        return a.order < b.order;
    });
    return keys;
}

bool KeyManager::reorderKeys(const std::vector<std::pair<String, int>>& newOrder) {
    LOG_INFO("KeyManager", "Reordering TOTP keys");
    
    // Создаем карту имя -> новый порядок
    std::map<String, int> orderMap;
    for (const auto& pair : newOrder) {
        orderMap[pair.first] = pair.second;
    }
    
    // Обновляем порядок для существующих ключей
    bool changed = false;
    for (auto& key : keys) {
        auto it = orderMap.find(key.name);
        if (it != orderMap.end() && key.order != it->second) {
            key.order = it->second;
            changed = true;
        }
    }
    
    if (changed) {
        bool success = saveKeys();
        if (success) {
            LOG_INFO("KeyManager", "Successfully reordered TOTP keys");
        } else {
            LOG_ERROR("KeyManager", "Failed to save reordered keys");
        }
        return success;
    }
    
    return true; // Никаких изменений не было
}

// --- Новая функция для импорта ---
bool KeyManager::replaceAllKeys(const String& jsonContent) {
    LOG_INFO("KeyManager", "Importing TOTP keys from JSON");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error) {
        LOG_ERROR("KeyManager", "Import failed, invalid JSON: " + String(error.c_str()));
        return false;
    }

    keys.clear();
    JsonArray array = doc.as<JsonArray>();
    int currentOrder = 0;
    for (JsonObject obj : array) {
        TOTPKey key;
        key.name = obj["name"].as<String>();
        key.secret = obj["secret"].as<String>();
        key.order = obj["order"] | currentOrder++;  // Используем существующий order или назначаем по порядку
        keys.push_back(key);
    }

    // Сохраняем новый набор ключей, который будет автоматически зашифрован
    bool success = saveKeys();
    if (success) {
        LOG_INFO("KeyManager", "Successfully imported " + String(keys.size()) + " TOTP keys");
    } else {
        LOG_ERROR("KeyManager", "Failed to save imported keys");
    }
    return success;
}

bool KeyManager::loadKeys() {
    LOG_DEBUG("KeyManager", "Loading TOTP keys from file");
    if (!LittleFS.exists(KEYS_FILE)) {
        LOG_INFO("KeyManager", "Keys file doesn't exist yet, starting with empty list");
        return true;
    }

    File file = LittleFS.open(KEYS_FILE, "r");
    if (!file) {
        LOG_ERROR("KeyManager", "Failed to open keys file for reading");
        return false;
    }

    String encrypted_base64 = file.readString();
    file.close();

    if (encrypted_base64.length() == 0) {
        keys.clear();
        LOG_INFO("KeyManager", "Keys file is empty");
        return true;
    }

    String json_string = CryptoManager::getInstance().decrypt(encrypted_base64);
    if (json_string.length() == 0) {
        LOG_ERROR("KeyManager", "Failed to decrypt keys file");
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json_string);
    if (error) {
        LOG_ERROR("KeyManager", "JSON parsing failed for keys: " + String(error.c_str()));
        return false;
    }

    keys.clear();
    JsonArray array = doc.as<JsonArray>();
    int currentOrder = 0;
    for (JsonObject obj : array) {
        TOTPKey key;
        key.name = obj["name"].as<String>(); 
        key.secret = obj["secret"].as<String>();
        key.order = obj["order"] | currentOrder++;  // Используем существующий order или назначаем по порядку
        keys.push_back(key);
    }
    LOG_INFO("KeyManager", "Loaded " + String(keys.size()) + " TOTP keys successfully");
    return true;
}

bool KeyManager::saveKeys() {
    LOG_DEBUG("KeyManager", "Saving TOTP keys to file");
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    for (const auto& key : keys) {
        JsonObject obj = array.add<JsonObject>();
        obj["name"] = key.name;
        obj["secret"] = key.secret;
        obj["order"] = key.order;
    }
    
    String json_string;
    size_t jsonSize = serializeJson(doc, json_string);
    if (jsonSize == 0) {
        LOG_ERROR("KeyManager", "Failed to serialize keys to JSON");
        return false;
    }

    String encrypted_base64 = CryptoManager::getInstance().encrypt(json_string);
    if (encrypted_base64.length() == 0) {
        LOG_ERROR("KeyManager", "Failed to encrypt keys");
        return false;
    }

    File file = LittleFS.open(KEYS_FILE, "w");
    if (!file) {
        LOG_ERROR("KeyManager", "Failed to open keys file for writing");
        return false;
    }
    
    size_t bytesWritten = file.print(encrypted_base64);
    file.close();
    
    if (bytesWritten > 0) {
        LOG_INFO("KeyManager", "Saved " + String(keys.size()) + " TOTP keys successfully");
        return true;
    } else {
        LOG_ERROR("KeyManager", "Failed to write encrypted keys data");
        return false;
    }
}
