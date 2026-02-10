#ifndef CRYPTO_MANAGER_H
#define CRYPTO_MANAGER_H

#include <Arduino.h>
#include <vector>
#include "LittleFS.h"

#define DEVICE_KEY_FILE "/device.key"

class CryptoManager {
public:
    static CryptoManager& getInstance();
    void begin();

    // --- Password Hashing ---
    String hashPassword(const String& password);
    bool verifyPassword(const String& password, const String& hash);

    // --- Base64 Encoding/Decoding ---
    String base64Encode(const uint8_t* data, size_t len);
    std::vector<uint8_t> base64Decode(const String& encoded);

    // --- Symmetric Encryption/Decryption (for files) ---
    String encrypt(const String& plaintext);
    String decrypt(const String& base64_ciphertext);

    // --- New Password-based Encryption for Import/Export ---
    String encryptWithPassword(const String& plaintext, const String& password);
    String decryptWithPassword(const String& encryptedJson, const String& password);
    
    // Session ID generation
    String generateSecureSessionId();
    
    // CSRF token generation
    String generateCsrfToken();
    
    // Client ID generation for secure sessions
    String generateClientId(const String& fingerprint);
    
    // --- Persistent Session Management ---
    bool saveSession(const String& sessionId, const String& csrfToken, unsigned long createdTime);
    bool loadSession(String& sessionId, String& csrfToken, unsigned long& createdTime);
    bool clearSession();
    bool isSessionValid(unsigned long createdTime, unsigned long maxLifetimeSeconds = 21600); // Default 6 hours
    bool isSessionValidEpoch(unsigned long epochCreatedTime, unsigned long maxLifetimeSeconds = 21600); // Epoch-based validation default

    bool encryptData(const uint8_t* plain, size_t plain_len, std::vector<uint8_t>& output);
    bool decryptData(const uint8_t* encrypted, size_t encrypted_len, std::vector<uint8_t>& output);

    // --- BLE PIN Management ---
    bool saveBlePin(uint32_t pin);
    uint32_t loadBlePin();
    bool isBlePinConfigured();
    uint32_t generateSecurePin();

private:
    CryptoManager(); // Private constructor
    CryptoManager(const CryptoManager&) = delete;
    void operator=(const CryptoManager&) = delete;

    unsigned char _deviceKey[32]; // 256-bit AES key
    bool _isKeyInitialized;

    void generateAndSaveKey();
    void loadKey();
};

#endif // CRYPTO_MANAGER_H

