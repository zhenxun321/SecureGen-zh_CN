#ifndef DEVICE_STATIC_KEY_H
#define DEVICE_STATIC_KEY_H

#include <Arduino.h>
#include "crypto_manager.h"
#include "log_manager.h"

/**
 * @brief DeviceStaticKey - Генерация уникального статичного ключа на основе ESP32 hardware
 * 
 * Используется для защиты ECDH handshake через Password-Based Key Wrapping.
 * Ключ генерируется из уникальных характеристик ESP32 чипа и остается постоянным
 * для конкретного устройства на протяжении всего жизненного цикла.
 */
class DeviceStaticKey {
public:
    static DeviceStaticKey& getInstance();
    
    // Инициализация и генерация device key
    bool begin();
    
    // Получение device static key (Base64 encoded SHA256 hash)
    String getDeviceStaticKey();
    
    // Проверка валидности device key
    bool isKeyInitialized() const;
    
    // Принудительная регенерация ключа (только для отладки)
    void regenerateKey();
    
    // Получение отладочной информации о hardware
    String getHardwareInfo();

private:
    DeviceStaticKey();
    ~DeviceStaticKey();
    DeviceStaticKey(const DeviceStaticKey&) = delete;
    DeviceStaticKey& operator=(const DeviceStaticKey&) = delete;
    
    // Генерация device-specific ключа
    String generateDeviceStaticKey();
    
    // Сбор ESP32 hardware identifiers
    void collectHardwareIdentifiers(uint8_t* buffer, size_t bufferSize);
    
    // Внутреннее состояние
    String deviceStaticKey;
    bool keyInitialized;
    
    // Hardware info для отладки
    struct HardwareInfo {
        uint64_t chipId;
        uint32_t flashId;
        uint32_t chipRevision;
        String chipModel;
        uint32_t flashSize;
    } hardwareInfo;
};

#endif // DEVICE_STATIC_KEY_H
