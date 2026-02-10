#include "crypto_manager.h"
#include "config.h" // <-- ADDED for BLE PIN constants
#include "config_manager.h" // <-- ADDED for session duration
#include "log_manager.h"
#include "mbedtls/sha256.h"
#include "mbedtls/base64.h"
#include "mbedtls/aes.h"
#include "mbedtls/pkcs5.h" // For PBKDF2
#include <esp_system.h>
#include <esp_task_wdt.h> // <-- ADDED for watchdog reset during PBKDF2
#include <ArduinoJson.h> // <-- ADDED for new functions

// --- New Password-based Encryption for Import/Export ---

String CryptoManager::encryptWithPassword(const String& plaintext, const String& password) {
    LOG_DEBUG("CryptoManager", "Encrypting data with user password.");
    const int salt_len = 16;
    const int key_len = 32; // 256-bit derived key
    const int iterations = PBKDF2_ITERATIONS_EXPORT; // Используем константу из config.h (100,000 iterations)

    LOG_INFO("CryptoManager", "Deriving encryption key with PBKDF2 (" + String(iterations) + " iterations)...");
    unsigned long start_time = millis();

    // 1. Generate random salt and IV
    uint8_t salt[salt_len];
    uint8_t iv[16];
    for (int i = 0; i < salt_len; i++) salt[i] = esp_random() & 0xFF;
    for (int i = 0; i < 16; i++) iv[i] = esp_random() & 0xFF;
    
    uint8_t iv_copy[16];
    memcpy(iv_copy, iv, 16);

    // 2. Derive encryption key from password and salt
    // ⚠️ Сбрасываем watchdog перед PBKDF2 (~3s)
    esp_task_wdt_reset();
    
    uint8_t derived_key[key_len];
    mbedtls_md_context_t sha256_ctx;
    mbedtls_md_init(&sha256_ctx);
    mbedtls_md_setup(&sha256_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_pkcs5_pbkdf2_hmac(&sha256_ctx, (const unsigned char*)password.c_str(), password.length(), salt, salt_len, iterations, key_len, derived_key);
    mbedtls_md_free(&sha256_ctx);
    
    esp_task_wdt_reset(); // Сбрасываем после PBKDF2
    
    unsigned long pbkdf2_elapsed = millis() - start_time;
    LOG_INFO("CryptoManager", "Key derivation completed in " + String(pbkdf2_elapsed) + "ms");

    // 3. Pad plaintext (PKCS7)
    size_t plain_len = plaintext.length();
    size_t padding_len = 16 - (plain_len % 16);
    size_t padded_len = plain_len + padding_len;
    std::vector<uint8_t> padded_input(padded_len);
    memcpy(padded_input.data(), plaintext.c_str(), plain_len);
    for(size_t i = 0; i < padding_len; i++) {
        padded_input[plain_len + i] = padding_len;
    }

    // 4. Encrypt data
    std::vector<uint8_t> ciphertext(padded_len);
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, derived_key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded_input.data(), ciphertext.data());
    mbedtls_aes_free(&aes);

    // 5. Package salt, IV, and ciphertext into a JSON object
    JsonDocument doc;
    doc["salt"] = base64Encode(salt, salt_len);
    doc["iv"] = base64Encode(iv, 16);
    doc["ciphertext"] = base64Encode(ciphertext.data(), ciphertext.size());

    String output;
    serializeJson(doc, output);
    LOG_INFO("CryptoManager", "Data successfully encrypted for export.");
    return output;
}

String CryptoManager::decryptWithPassword(const String& encryptedJson, const String& password) {
    LOG_DEBUG("CryptoManager", "Decrypting data with user password.");
    // 🛡️ Буфер для export/import данных (может быть большой файл)
    DynamicJsonDocument doc(2048); // 2KB для {salt, iv, ciphertext}
    DeserializationError error = deserializeJson(doc, encryptedJson);
    if (error) {
        LOG_ERROR("CryptoManager", "Failed to parse encrypted JSON: " + String(error.c_str()));
        return "";
    }

    if (!doc["salt"].is<String>() || !doc["iv"].is<String>() || !doc["ciphertext"].is<String>()) {
        LOG_ERROR("CryptoManager", "Encrypted JSON is missing required fields (salt, iv, ciphertext).");
        return "";
    }

    // 1. Decode all components from Base64
    std::vector<uint8_t> salt = base64Decode(doc["salt"].as<String>());
    std::vector<uint8_t> iv = base64Decode(doc["iv"].as<String>());
    std::vector<uint8_t> ciphertext = base64Decode(doc["ciphertext"].as<String>());

    if (salt.empty() || iv.empty() || ciphertext.empty() || iv.size() != 16 || ciphertext.size() % 16 != 0) {
        LOG_ERROR("CryptoManager", "Invalid data format after Base64 decoding.");
        return "";
    }

    // 2. Re-derive the key using the provided password and the extracted salt
    const int key_len = 32;
    const int iterations = PBKDF2_ITERATIONS_EXPORT; // Используем константу из config.h (100,000 iterations)
    
    LOG_INFO("CryptoManager", "Deriving decryption key with PBKDF2 (" + String(iterations) + " iterations)...");
    unsigned long start_time = millis();
    
    // ⚠️ Сбрасываем watchdog перед PBKDF2 (~3s)
    esp_task_wdt_reset();
    
    uint8_t derived_key[key_len];
    mbedtls_md_context_t sha256_ctx;
    mbedtls_md_init(&sha256_ctx);
    mbedtls_md_setup(&sha256_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_pkcs5_pbkdf2_hmac(&sha256_ctx, (const unsigned char*)password.c_str(), password.length(), salt.data(), salt.size(), iterations, key_len, derived_key);
    mbedtls_md_free(&sha256_ctx);
    
    esp_task_wdt_reset(); // Сбрасываем после PBKDF2
    
    unsigned long pbkdf2_elapsed = millis() - start_time;
    LOG_INFO("CryptoManager", "Key derivation completed in " + String(pbkdf2_elapsed) + "ms");

    // 3. Decrypt data
    std::vector<uint8_t> decrypted_padded(ciphertext.size());
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, derived_key, 256);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext.size(), iv.data(), ciphertext.data(), decrypted_padded.data());
    mbedtls_aes_free(&aes);

    // 4. Unpad data (PKCS7)
    if(decrypted_padded.empty()) return "";
    uint8_t padding_len = decrypted_padded.back();
    if (padding_len > 16 || padding_len == 0 || padding_len > decrypted_padded.size()) {
        LOG_ERROR("CryptoManager", "Decryption failed: Invalid padding length.");
        return "";
    }
    for(size_t i = 0; i < padding_len; ++i) {
        if (decrypted_padded[decrypted_padded.size() - 1 - i] != padding_len) {
            LOG_ERROR("CryptoManager", "Decryption failed: Padding verification failed.");
            return "";
        }
    }

    size_t plain_len = decrypted_padded.size() - padding_len;
    LOG_INFO("CryptoManager", "Data successfully decrypted from import.");
    return String((char*)decrypted_padded.data(), plain_len);
}


CryptoManager& CryptoManager::getInstance() {
    static CryptoManager instance;
    return instance;
}

CryptoManager::CryptoManager() : _isKeyInitialized(false) {}

void CryptoManager::begin() {
    if (_isKeyInitialized) return;

    LOG_INFO("CryptoManager", "Initializing...");
    if (LittleFS.exists(DEVICE_KEY_FILE)) {
        loadKey();
    } else {
        generateAndSaveKey();
    }
    _isKeyInitialized = true;
    LOG_INFO("CryptoManager", "Initialized successfully");
}

void CryptoManager::generateAndSaveKey() {
    LOG_INFO("CryptoManager", "Generating new device key...");
    for (int i = 0; i < sizeof(_deviceKey); i++) {
        _deviceKey[i] = esp_random() & 0xFF;
    }

    fs::File keyFile = LittleFS.open(DEVICE_KEY_FILE, "w");
    if (keyFile) {
        keyFile.write(_deviceKey, sizeof(_deviceKey));
        keyFile.close();
        LOG_INFO("CryptoManager", "New key saved to file");
    } else {
        LOG_CRITICAL("CryptoManager", "Failed to save new key!");
    }
}

void CryptoManager::loadKey() {
    LOG_INFO("CryptoManager", "Loading device key from file...");
    fs::File keyFile = LittleFS.open(DEVICE_KEY_FILE, "r");
    if (keyFile && keyFile.size() == sizeof(_deviceKey)) {
        keyFile.read(_deviceKey, sizeof(_deviceKey));
        keyFile.close();
        LOG_INFO("CryptoManager", "Key loaded successfully");
    } else {
        LOG_CRITICAL("CryptoManager", "Invalid key file size or failed to open!");
        // If key is invalid or corrupted, generate a new one.
        // This will make old data unreadable, but prevents a bricked state.
        generateAndSaveKey();
    }
}

#include "mbedtls/pkcs5.h"
#include "mbedtls/md.h"

// --- Password Hashing (PBKDF2) ---

// Helper to convert byte array to hex string
static String bytesToHex(const uint8_t* data, size_t len) {
    String hexStr = "";
    for (size_t i = 0; i < len; i++) {
        char hex[3];
        sprintf(hex, "%02x", data[i]);
        hexStr += hex;
    }
    return hexStr;
}

// Helper to convert hex string to byte array
static std::vector<uint8_t> hexToBytes(const String& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        String byteString = hex.substring(i, i + 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

String CryptoManager::hashPassword(const String& password) {
    const int salt_len = 16;
    const int key_len = 32; // 256-bit derived key
    const int iterations = PBKDF2_ITERATIONS_LOGIN; // Используем константу из config.h (50,000 iterations)

    LOG_INFO("CryptoManager", "Hashing password with PBKDF2 (" + String(iterations) + " iterations)...");
    unsigned long start_time = millis();

    // 1. Generate a random salt
    uint8_t salt[salt_len];
    for (int i = 0; i < salt_len; i++) {
        salt[i] = esp_random() & 0xFF;
    }

    // 2. Derive the key using PBKDF2
    // ⚠️ Временно отключаем watchdog, так как PBKDF2 может занять 2+ секунды
    esp_task_wdt_reset();
    
    uint8_t derived_key[key_len];
    mbedtls_md_context_t sha256_ctx;
    mbedtls_md_init(&sha256_ctx);
    mbedtls_md_setup(&sha256_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1); // HMAC

    mbedtls_pkcs5_pbkdf2_hmac(
        &sha256_ctx,
        (const unsigned char*)password.c_str(), password.length(),
        salt, salt_len,
        iterations,
        key_len,
        derived_key
    );

    mbedtls_md_free(&sha256_ctx);
    
    // Сбрасываем watchdog после PBKDF2
    esp_task_wdt_reset();
    
    unsigned long elapsed = millis() - start_time;
    LOG_INFO("CryptoManager", "Password hashed in " + String(elapsed) + "ms");

    // 3. Combine salt and key into "salt:key" format
    String salt_hex = bytesToHex(salt, salt_len);
    String key_hex = bytesToHex(derived_key, key_len);

    return salt_hex + ":" + key_hex;
}

bool CryptoManager::verifyPassword(const String& password, const String& salt_and_hash) {
    int separator_index = salt_and_hash.indexOf(':');
    if (separator_index == -1) return false;

    // 1. Extract salt and original hash
    String salt_hex = salt_and_hash.substring(0, separator_index);
    String original_hash_hex = salt_and_hash.substring(separator_index + 1);

    std::vector<uint8_t> salt = hexToBytes(salt_hex);
    
    const int key_len = 32;
    const int iterations = PBKDF2_ITERATIONS_LOGIN; // Используем константу из config.h (50,000 iterations)

    LOG_DEBUG("CryptoManager", "Verifying password with PBKDF2 (" + String(iterations) + " iterations)...");
    unsigned long start_time = millis();

    // ⚠️ Сбрасываем watchdog перед длительной операцией PBKDF2
    esp_task_wdt_reset();

    // 2. Derive a key from the provided password and the extracted salt
    uint8_t derived_key[key_len];
    mbedtls_md_context_t sha256_ctx;
    mbedtls_md_init(&sha256_ctx);
    mbedtls_md_setup(&sha256_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1); // HMAC

    mbedtls_pkcs5_pbkdf2_hmac(
        &sha256_ctx,
        (const unsigned char*)password.c_str(), password.length(),
        salt.data(), salt.size(),
        iterations,
        key_len,
        derived_key
    );

    mbedtls_md_free(&sha256_ctx);
    
    // Сбрасываем watchdog после PBKDF2
    esp_task_wdt_reset();
    
    unsigned long elapsed = millis() - start_time;
    LOG_DEBUG("CryptoManager", "Password verification completed in " + String(elapsed) + "ms");

    // 3. Compare the new key with the original one
    String derived_key_hex = bytesToHex(derived_key, key_len);
    
    return derived_key_hex.equals(original_hash_hex);
}

// --- Base64 Encoding/Decoding ---

String CryptoManager::base64Encode(const uint8_t* data, size_t len) {
    if (len == 0) return "";

    size_t output_len;
    mbedtls_base64_encode(NULL, 0, &output_len, data, len);

    // Безопасное выделение памяти с автоматическим освобождением
    std::vector<uint8_t> encoded_buf(output_len);
    
    mbedtls_base64_encode(encoded_buf.data(), output_len, &output_len, data, len);
    
    String encoded_str = String((char*)encoded_buf.data());
    return encoded_str;
}

std::vector<uint8_t> CryptoManager::base64Decode(const String& encoded) {
    std::vector<uint8_t> result;
    if (encoded.length() == 0) {
        return result;
    }
    
    size_t output_len;
    mbedtls_base64_decode(NULL, 0, &output_len, (const unsigned char*)encoded.c_str(), encoded.length());

    result.resize(output_len);

    int ret = mbedtls_base64_decode(result.data(), output_len, &output_len, (const unsigned char*)encoded.c_str(), encoded.length());
    if (ret != 0) {
        result.clear();
    }
    result.resize(output_len); // Resize to actual decoded length
    return result;
}


// --- Symmetric Encryption/Decryption ---

bool CryptoManager::encryptData(const uint8_t* plain, size_t plain_len, std::vector<uint8_t>& output) {
    if (!_isKeyInitialized) return false;
    
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, _deviceKey, 256);

    // --- IV Generation ---
    unsigned char iv[16];
    for (int i = 0; i < 16; i++) {
        iv[i] = esp_random() & 0xFF;
    }
    unsigned char iv_copy[16];
    memcpy(iv_copy, iv, 16); // mbedtls_aes_crypt_cbc modifies the IV, so we need a copy

    // PKCS7 Padding
    size_t padding_len = 16 - (plain_len % 16);
    size_t padded_len = plain_len + padding_len;
    
    std::vector<uint8_t> padded_input(padded_len);
    memcpy(padded_input.data(), plain, plain_len);
    for(size_t i = 0; i < padding_len; i++) {
        padded_input[plain_len + i] = padding_len;
    }

    // The output will be [IV] + [Ciphertext]
    output.resize(16 + padded_len);
    memcpy(output.data(), iv, 16); // Prepend the IV

    // Encrypt the padded data
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded_input.data(), output.data() + 16);
    
    mbedtls_aes_free(&aes);
    return true;
}

bool CryptoManager::decryptData(const uint8_t* encrypted, size_t encrypted_len, std::vector<uint8_t>& output) {
    // Must be at least 16 bytes for IV + one block of data, and a multiple of 16
    if (encrypted_len < 32 || encrypted_len % 16 != 0 || !_isKeyInitialized) return false;

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, _deviceKey, 256);

    // Extract IV from the beginning of the data
    unsigned char iv[16];
    memcpy(iv, encrypted, 16);

    const uint8_t* ciphertext = encrypted + 16;
    size_t ciphertext_len = encrypted_len - 16;

    std::vector<uint8_t> decrypted_padded(ciphertext_len);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext_len, iv, ciphertext, decrypted_padded.data());
    mbedtls_aes_free(&aes);

    // PKCS7 Unpadding
    if(decrypted_padded.empty()) return false;
    uint8_t padding_len = decrypted_padded.back();
    if (padding_len > 16 || padding_len == 0) return false;
    if (padding_len > decrypted_padded.size()) return false;

    // Verify padding bytes
    for(size_t i = 0; i < padding_len; ++i) {
        if (decrypted_padded[decrypted_padded.size() - 1 - i] != padding_len) {
            return false; // Invalid padding
        }
    }

    size_t plain_len = ciphertext_len - padding_len;
    output.assign(decrypted_padded.begin(), decrypted_padded.begin() + plain_len);
    
    return true;
}

String CryptoManager::encrypt(const String& plaintext) {
    std::vector<uint8_t> encrypted_buffer;
    if (!encryptData((const uint8_t*)plaintext.c_str(), plaintext.length(), encrypted_buffer)) {
        return "";
    }
    return base64Encode(encrypted_buffer.data(), encrypted_buffer.size());
}

String CryptoManager::decrypt(const String& base64_ciphertext) {
    std::vector<uint8_t> encrypted_buffer = base64Decode(base64_ciphertext);
    if (encrypted_buffer.empty()) {
        return "";
    }

    std::vector<uint8_t> decrypted_buffer;
    if (!decryptData(encrypted_buffer.data(), encrypted_buffer.size(), decrypted_buffer)) {
        return "";
    }

    return String((char*)decrypted_buffer.data(), decrypted_buffer.size());
}

// --- BLE PIN Management ---
bool CryptoManager::saveBlePin(uint32_t pin) {
    if (!_isKeyInitialized) {
        Serial.println("[CryptoManager] Error: Key not initialized for BLE PIN encryption");
        return false;
    }

    try {
        JsonDocument doc;
        doc["ble_pin"] = pin;
        doc["timestamp"] = millis();
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        
        String encrypted = encrypt(jsonStr);
        if (encrypted.isEmpty()) {
            Serial.println("[CryptoManager] Error: Failed to encrypt BLE PIN");
            return false;
        }
        
        File file = LittleFS.open("/ble_pin.json.enc", "w");
        if (!file) {
            Serial.println("[CryptoManager] Error: Cannot create BLE PIN file");
            return false;
        }
        
        file.print(encrypted);
        file.close();
        
        Serial.println("[CryptoManager] BLE PIN saved and encrypted successfully");
        return true;
        
    } catch (const std::exception& e) {
        Serial.println("[CryptoManager] Exception saving BLE PIN: " + String(e.what()));
        return false;
    }
}

uint32_t CryptoManager::loadBlePin() {
    if (!LittleFS.exists("/ble_pin.json.enc")) {
        LOG_INFO("CryptoManager", "BLE PIN file not found - first boot detected");
        
        #if BLE_PIN_AUTO_GENERATE
            // Генерируем новый случайный PIN при первом запуске
            uint32_t newPin = generateSecurePin();
            
            // Сохраняем сгенерированный PIN
            if (saveBlePin(newPin)) {
                LOG_INFO("CryptoManager", "New secure BLE PIN generated and saved on first boot");
                return newPin;
            } else {
                LOG_ERROR("CryptoManager", "Failed to save generated PIN, using fallback");
                return 123456; // Fallback если не удалось сохранить
            }
        #else
            LOG_INFO("CryptoManager", "Auto-generation disabled, using default PIN");
            return 123456; // Default PIN если автогенерация отключена
        #endif
    }
    
    try {
        File file = LittleFS.open("/ble_pin.json.enc", "r");
        if (!file) {
            LOG_ERROR("CryptoManager", "Cannot open BLE PIN file");
            return 123456; // Default PIN
        }
        
        String encrypted = file.readString();
        file.close();
        
        String decrypted = decrypt(encrypted);
        if (decrypted.isEmpty()) {
            LOG_ERROR("CryptoManager", "Failed to decrypt BLE PIN");
            return 123456; // Default PIN
        }
        
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, decrypted);
        if (error) {
            LOG_ERROR("CryptoManager", "Failed to parse BLE PIN JSON");
            return 123456; // Default PIN
        }
        
        uint32_t pin = doc["ble_pin"] | 123456;
        LOG_INFO("CryptoManager", "BLE PIN loaded successfully from encrypted storage");
        return pin;
        
    } catch (const std::exception& e) {
        LOG_ERROR("CryptoManager", "Exception loading BLE PIN: " + String(e.what()));
        return 123456; // Default PIN
    }
}

bool CryptoManager::isBlePinConfigured() {
    return LittleFS.exists("/ble_pin.json.enc");
}

uint32_t CryptoManager::generateSecurePin() {
    LOG_INFO("CryptoManager", "Generating secure random BLE PIN...");
    
    // Генерируем случайный PIN в диапазоне от BLE_PIN_MIN_VALUE до BLE_PIN_MAX_VALUE
    uint32_t randomPin = BLE_PIN_MIN_VALUE + (esp_random() % (BLE_PIN_MAX_VALUE - BLE_PIN_MIN_VALUE + 1));
    
    // Проверяем, что PIN соответствует требованиям длины
    String pinStr = String(randomPin);
    if (pinStr.length() != BLE_PIN_LENGTH) {
        LOG_WARNING("CryptoManager", "Generated PIN length mismatch, regenerating...");
        // Принудительно генерируем PIN правильной длины
        randomPin = BLE_PIN_MIN_VALUE + (esp_random() % (BLE_PIN_MAX_VALUE - BLE_PIN_MIN_VALUE + 1));
    }
    
    LOG_INFO("CryptoManager", "Secure BLE PIN generated successfully (length: " + String(String(randomPin).length()) + ")");
    return randomPin;
}

String CryptoManager::generateSecureSessionId() {
    // Generate 128-bit (16 bytes) cryptographically secure session ID
    uint8_t sessionBytes[16];
    for (int i = 0; i < 16; i++) {
        sessionBytes[i] = esp_random() & 0xFF;
    }
    
    // Convert to hex string (32 characters)
    String sessionId = "";
    for (int i = 0; i < 16; i++) {
        if (sessionBytes[i] < 16) sessionId += "0";
        sessionId += String(sessionBytes[i], HEX);
    }
    
    LOG_INFO("CryptoManager", "Generated secure session ID (128-bit)");
    return sessionId;
}

String CryptoManager::generateCsrfToken() {
    LOG_DEBUG("CryptoManager", "Generating new CSRF token");
    
    const int token_len = 32; // 256-bit token
    uint8_t token_bytes[token_len];
    
    // Generate random bytes
    for (int i = 0; i < token_len; i++) {
        token_bytes[i] = esp_random() & 0xFF;
    }
    
    // Convert to hex string
    String token = "";
    for (int i = 0; i < token_len; i++) {
        char hex[3];
        sprintf(hex, "%02x", token_bytes[i]);
        token += hex;
    }
    
    LOG_DEBUG("CryptoManager", "CSRF token generated, length: " + String(token.length()));
    return token;
}

String CryptoManager::generateClientId(const String& fingerprint) {
    LOG_DEBUG("CryptoManager", "Generating client ID from fingerprint");
    
    // Hash fingerprint to create consistent client ID
    const uint8_t* input = (const uint8_t*)fingerprint.c_str();
    size_t input_len = fingerprint.length();
    uint8_t hash[32]; // SHA-256 output
    
    // Compute SHA-256 hash of fingerprint
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA-256
    mbedtls_sha256_update(&ctx, input, input_len);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
    
    // Convert first 16 bytes to hex string for client ID
    String clientId = "";
    for (int i = 0; i < 16; i++) {
        char hex[3];
        sprintf(hex, "%02x", hash[i]);
        clientId += hex;
    }
    
    LOG_DEBUG("CryptoManager", "Client ID generated: " + clientId.substring(0,8) + "...");
    return clientId;
}

bool CryptoManager::saveSession(const String& sessionId, const String& csrfToken, unsigned long createdTime) {
    if (!_isKeyInitialized) {
        LOG_ERROR("CryptoManager", "Cannot save session: crypto not initialized");
        return false;
    }
    
    // ИСПРАВЛЕНО: Сохраняем epoch time вместо millis() для персистентности
    struct tm timeinfo;
    time_t now;
    time(&now);
    
    JsonDocument doc;
    doc["session_id"] = sessionId;
    doc["csrf_token"] = csrfToken;
    doc["created_time_millis"] = createdTime; // Сохраняем millis() для совместимости
    doc["created_time_epoch"] = (unsigned long)now; // Добавляем epoch time
    doc["version"] = 2; // Увеличиваем версию
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Encrypt and save
    String encryptedSession = encrypt(jsonString);
    if (encryptedSession.isEmpty()) {
        LOG_ERROR("CryptoManager", "Failed to encrypt session data");
        return false;
    }
    
    File file = LittleFS.open("/session.json.enc", "w");
    if (!file) {
        LOG_ERROR("CryptoManager", "Failed to open session file for writing");
        return false;
    }
    
    file.print(encryptedSession);
    file.close();
    
    LOG_INFO("CryptoManager", "Session saved to encrypted flash storage with epoch time");
    return true;
}

bool CryptoManager::loadSession(String& sessionId, String& csrfToken, unsigned long& createdTime) {
    if (!_isKeyInitialized) {
        LOG_ERROR("CryptoManager", "Cannot load session: crypto not initialized");
        return false;
    }
    
    // Check session duration mode - if "until reboot", don't load persistent session
    ConfigManager configManager;
    unsigned long sessionLifetime = configManager.getSessionLifetimeSeconds();
    if (sessionLifetime == 0) {
        LOG_DEBUG("CryptoManager", "Session mode: until reboot - not loading persistent session after reboot");
        return false;
    }
    
    if (!LittleFS.exists("/session.json.enc")) {
        LOG_DEBUG("CryptoManager", "No persistent session file found");
        return false;
    }
    
    File sessionFile = LittleFS.open("/session.json.enc", "r");
    if (!sessionFile) {
        LOG_ERROR("CryptoManager", "Failed to open session file");
        return false;
    }
    
    String encryptedData = sessionFile.readString();
    sessionFile.close();
    
    // Decrypt session data
    String decryptedJson = decrypt(encryptedData);
    if (decryptedJson.isEmpty()) {
        LOG_ERROR("CryptoManager", "Failed to decrypt session data");
        clearSession(); // Remove corrupted session
        return false;
    }
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, decryptedJson);
    if (error) {
        LOG_ERROR("CryptoManager", "Failed to parse session JSON: " + String(error.c_str()));
        clearSession(); // Remove corrupted session
        return false;
    }
    
    // Extract session data
    sessionId = doc["session_id"].as<String>();
    csrfToken = doc["csrf_token"].as<String>();
    createdTime = doc["created_time_millis"].as<unsigned long>(); // Для совместимости с web_server.cpp
    
    // Проверяем версию для выбора алгоритма валидации
    int version = doc["version"].as<int>();
    unsigned long epochCreatedTime = 0;
    
    if (version >= 2 && doc.containsKey("created_time_epoch")) {
        epochCreatedTime = doc["created_time_epoch"].as<unsigned long>();
    }
    
    // Validate session data
    if (sessionId.isEmpty() || csrfToken.isEmpty()) {
        LOG_ERROR("CryptoManager", "Invalid session data loaded");
        clearSession();
        return false;
    }
    
    // Check if session is still valid - используем epoch time если доступно
    bool sessionValid = false;
    if (epochCreatedTime > 0) {
        // Новый алгоритм: проверка по epoch time (переживает reboot)
        sessionValid = isSessionValidEpoch(epochCreatedTime);
        LOG_DEBUG("CryptoManager", "Validating session using epoch time");
    } else {
        // Старый алгоритм: проверка по millis() (не переживает reboot)
        sessionValid = isSessionValid(createdTime);
        LOG_DEBUG("CryptoManager", "Validating session using millis() time (legacy)");
    }
    
    if (!sessionValid) {
        LOG_INFO("CryptoManager", "Loaded session has expired, clearing");
        clearSession();
        return false;
    }
    
    LOG_INFO("CryptoManager", "Valid session loaded from encrypted storage");
    return true;
}

bool CryptoManager::clearSession() {
    if (LittleFS.exists("/session.json.enc")) {
        if (LittleFS.remove("/session.json.enc")) {
            LOG_INFO("CryptoManager", "Session file cleared from storage");
            return true;
        } else {
            LOG_ERROR("CryptoManager", "Failed to remove session file");
            return false;
        }
    }
    LOG_DEBUG("CryptoManager", "No session file to clear");
    return true;
}

bool CryptoManager::isSessionValid(unsigned long createdTime, unsigned long maxLifetimeSeconds) {
    unsigned long currentTime = millis();
    
    // ИСПРАВЛЕНО: Различаем reboot от настоящего millis() overflow
    if (currentTime < createdTime) {
        // Настоящий overflow происходит только после ~49 дней работы
        // При reboot currentTime будет маленьким (< 1 часа)
        const unsigned long ONE_HOUR_MS = 3600 * 1000;
        
        if (currentTime < ONE_HOUR_MS) {
            // Это reboot, не overflow - сессия могла быть создана до перезагрузки
            LOG_INFO("CryptoManager", "Device rebooted, session from previous boot - treating as expired");
            return false;
        } else {
            // Это настоящий overflow после долгой работы
            LOG_WARNING("CryptoManager", "millis() overflow detected, invalidating session for safety");
            return false;
        }
    }
    
    unsigned long sessionAge = (currentTime - createdTime) / 1000; // Convert to seconds
    bool isValid = sessionAge <= maxLifetimeSeconds;
    
    if (!isValid) {
        LOG_INFO("CryptoManager", "Session expired: age=" + String(sessionAge) + "s, max=" + String(maxLifetimeSeconds) + "s");
    }
    
    return isValid;
}

bool CryptoManager::isSessionValidEpoch(unsigned long epochCreatedTime, unsigned long maxLifetimeSeconds) {
    // ИСПРАВЛЕНО: Получаем актуальную длительность из конфига
    ConfigManager configManager;
    unsigned long actualLifetimeSeconds = configManager.getSessionLifetimeSeconds();
    
    // Специальный режим: сессия до ребута
    if (actualLifetimeSeconds == 0) {
        // В режиме "до ребута" сессия всегда валидна, если файл существует
        LOG_DEBUG("CryptoManager", "Session mode: valid until reboot");
        return true;
    }
    
    time_t now;
    time(&now);
    unsigned long currentEpoch = (unsigned long)now;
    
    // Проверяем что время синхронизировано (не 1970 год)
    if (currentEpoch < 1000000000) { // Примерно 2001 год
        LOG_WARNING("CryptoManager", "System time not synchronized, treating session as expired");
        return false;
    }
    
    unsigned long sessionAge = currentEpoch - epochCreatedTime;
    bool isValid = sessionAge <= actualLifetimeSeconds;
    
    if (!isValid) {
        LOG_INFO("CryptoManager", "Session expired: age=" + String(sessionAge) + "s, max=" + String(actualLifetimeSeconds) + "s (epoch-based)");
    } else {
        LOG_DEBUG("CryptoManager", "Session valid: age=" + String(sessionAge) + "s, remaining=" + String(actualLifetimeSeconds - sessionAge) + "s (epoch-based)");
    }
    
    return isValid;
}