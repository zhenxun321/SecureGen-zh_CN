#include "device_static_key.h"
#include "mbedtls/sha256.h"
#include <esp_system.h>

DeviceStaticKey& DeviceStaticKey::getInstance() {
    static DeviceStaticKey instance;
    return instance;
}

DeviceStaticKey::DeviceStaticKey() : keyInitialized(false) {
    // Constructor
}

DeviceStaticKey::~DeviceStaticKey() {
    // Destructor - secure cleanup
    deviceStaticKey.clear();
}

bool DeviceStaticKey::begin() {
    if (keyInitialized) {
        LOG_DEBUG("DeviceStaticKey", "Device key already initialized");
        return true;
    }
    
    LOG_INFO("DeviceStaticKey", "🔐 Initializing ESP32 hardware fingerprinting...");
    
    // Собираем hardware info для отладки
    hardwareInfo.chipId = ESP.getEfuseMac();
    hardwareInfo.flashId = ESP.getFlashChipMode(); // Используем FlashChipMode вместо deprecated FlashChipId
    hardwareInfo.chipRevision = ESP.getChipRevision();
    hardwareInfo.chipModel = ESP.getChipModel();
    hardwareInfo.flashSize = ESP.getFlashChipSize();
    
    // Генерируем device static key
    deviceStaticKey = generateDeviceStaticKey();
    
    if (deviceStaticKey.isEmpty()) {
        LOG_ERROR("DeviceStaticKey", "Failed to generate device static key");
        return false;
    }
    
    keyInitialized = true;
    
    LOG_INFO("DeviceStaticKey", "✅ Device static key generated successfully");
    LOG_DEBUG("DeviceStaticKey", "Chip ID: " + String((uint32_t)(hardwareInfo.chipId >> 32), HEX) + 
                                 String((uint32_t)hardwareInfo.chipId, HEX));
    LOG_DEBUG("DeviceStaticKey", "Flash ID: " + String(hardwareInfo.flashId, HEX));
    LOG_DEBUG("DeviceStaticKey", "Chip Model: " + hardwareInfo.chipModel);
    
    return true;
}

String DeviceStaticKey::getDeviceStaticKey() {
    if (!keyInitialized) {
        if (!begin()) {
            LOG_ERROR("DeviceStaticKey", "Failed to initialize device key on demand");
            return "";
        }
    }
    return deviceStaticKey;
}

bool DeviceStaticKey::isKeyInitialized() const {
    return keyInitialized;
}

void DeviceStaticKey::regenerateKey() {
    LOG_WARNING("DeviceStaticKey", "🔄 Forcing device key regeneration (DEBUG ONLY)");
    keyInitialized = false;
    deviceStaticKey.clear();
    begin();
}

String DeviceStaticKey::getHardwareInfo() {
    if (!keyInitialized) {
        begin();
    }
    
    String info = "ESP32 Hardware Info:\n";
    info += "- Chip ID: " + String((uint32_t)(hardwareInfo.chipId >> 32), HEX) + 
            String((uint32_t)hardwareInfo.chipId, HEX) + "\n";
    info += "- Flash ID: " + String(hardwareInfo.flashId, HEX) + "\n";
    info += "- Chip Revision: " + String(hardwareInfo.chipRevision) + "\n";
    info += "- Chip Model: " + hardwareInfo.chipModel + "\n";
    info += "- Flash Size: " + String(hardwareInfo.flashSize) + " bytes\n";
    info += "- Device Key Length: " + String(deviceStaticKey.length()) + " chars";
    
    return info;
}

String DeviceStaticKey::generateDeviceStaticKey() {
    LOG_DEBUG("DeviceStaticKey", "Generating device-specific key from hardware identifiers");
    
    // Буфер для сбора всех hardware identifiers
    const size_t bufferSize = 128;
    uint8_t hardwareBuffer[bufferSize];
    
    // Собираем все hardware identifiers в один буфер
    collectHardwareIdentifiers(hardwareBuffer, bufferSize);
    
    // Хешируем collected data с помощью SHA-256
    mbedtls_sha256_context ctx;
    uint8_t hash[32]; // SHA-256 = 32 bytes
    
    mbedtls_sha256_init(&ctx);
    
    mbedtls_sha256_starts(&ctx, 0); // 0 = SHA-256 (не SHA-224) - void return в новой версии
    mbedtls_sha256_update(&ctx, hardwareBuffer, bufferSize); // void return в новой версии  
    mbedtls_sha256_finish(&ctx, hash); // void return в новой версии
    
    mbedtls_sha256_free(&ctx);
    
    // Конвертируем hash в Base64 для удобства использования
    String deviceKey = CryptoManager::getInstance().base64Encode(hash, 32);
    
    LOG_DEBUG("DeviceStaticKey", "Device key generated from " + String(bufferSize) + 
              " bytes of hardware data");
    LOG_DEBUG("DeviceStaticKey", "Device key length: " + String(deviceKey.length()) + " chars");
    
    return deviceKey;
}

void DeviceStaticKey::collectHardwareIdentifiers(uint8_t* buffer, size_t bufferSize) {
    // Очищаем буфер
    memset(buffer, 0, bufferSize);
    
    size_t offset = 0;
    
    // 1. ESP32 MAC Address (EFuse MAC) - 8 bytes
    uint64_t chipId = ESP.getEfuseMac();
    if (offset + sizeof(chipId) <= bufferSize) {
        memcpy(buffer + offset, &chipId, sizeof(chipId));
        offset += sizeof(chipId);
    }
    
    // 2. Flash Chip Mode - 4 bytes (используем Mode вместо deprecated ID)
    uint32_t flashMode = ESP.getFlashChipMode();
    if (offset + sizeof(flashMode) <= bufferSize) {
        memcpy(buffer + offset, &flashMode, sizeof(flashMode));
        offset += sizeof(flashMode);
    }
    
    // 3. Chip Revision - 4 bytes
    uint32_t chipRevision = ESP.getChipRevision();
    if (offset + sizeof(chipRevision) <= bufferSize) {
        memcpy(buffer + offset, &chipRevision, sizeof(chipRevision));
        offset += sizeof(chipRevision);
    }
    
    // 4. Flash Size - 4 bytes
    uint32_t flashSize = ESP.getFlashChipSize();
    if (offset + sizeof(flashSize) <= bufferSize) {
        memcpy(buffer + offset, &flashSize, sizeof(flashSize));
        offset += sizeof(flashSize);
    }
    
    // 5. Flash Speed - 4 bytes
    uint32_t flashSpeed = ESP.getFlashChipSpeed();
    if (offset + sizeof(flashSpeed) <= bufferSize) {
        memcpy(buffer + offset, &flashSpeed, sizeof(flashSpeed));
        offset += sizeof(flashSpeed);
    }
    
    // 6. Chip Model String - до 32 bytes
    String chipModel = ESP.getChipModel();
    size_t modelLen = min((size_t)chipModel.length(), (size_t)32);
    if (offset + modelLen <= bufferSize) {
        memcpy(buffer + offset, chipModel.c_str(), modelLen);
        offset += modelLen;
    }
    
    // 7. Static Salt для дополнительной энтропии - остальное место в буфере
    const char* staticSalt = "TOTP-DEVICE-STATIC-2024-ESP32-FINGERPRINT";
    size_t saltLen = strlen(staticSalt);
    size_t remainingSpace = bufferSize - offset;
    
    if (remainingSpace > 0) {
        size_t copyLen = min(saltLen, remainingSpace);
        memcpy(buffer + offset, staticSalt, copyLen);
        offset += copyLen;
    }
    
    LOG_DEBUG("DeviceStaticKey", "Collected " + String(offset) + " bytes of hardware identifiers");
}
