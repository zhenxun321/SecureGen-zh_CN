#include "pin_manager.h"
#include <FS.h>
#include "config.h"
#include "crypto_manager.h"
#include "log_manager.h"
#include <ArduinoJson.h>
#include "LittleFS.h"
#include <esp_task_wdt.h>

PinManager::PinManager(DisplayManager& display) : displayManager(display) {
    // Конструктор пуст
}

void PinManager::begin() {
    LOG_INFO("PinManager", "Initializing...");
    loadPinConfig();
    LOG_INFO("PinManager", "Initialized successfully");
}

void PinManager::loadPinConfig() {
    LOG_DEBUG("PinManager", "Loading PIN configuration");
    if (LittleFS.exists(PIN_FILE)) {
        fs::File configFile = LittleFS.open(PIN_FILE, "r");
        if (configFile) {
            String encryptedContent = configFile.readString();
            configFile.close();
            
            String decryptedContent = CryptoManager::getInstance().decrypt(encryptedContent);
            if (decryptedContent.isEmpty()) {
                LOG_ERROR("PinManager", "Failed to decrypt PIN config");
                return;
            }

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, decryptedContent);
            if (error == DeserializationError::Ok) {
                // Новый формат с отдельными настройками
                enabledForDevice = doc["enabledForDevice"] | false;
                enabledForBle = doc["enabledForBle"] | false;
                
                // Поддержка старого формата для обратной совместимости
                if (doc["enabled"].is<bool>() && !doc["enabledForDevice"].is<bool>()) {
                    bool oldEnabled = doc["enabled"] | false;
                    enabledForDevice = oldEnabled;
                    enabledForBle = oldEnabled;
                }
                
                pinHash = doc["hash"].as<String>();
                currentPinLength = doc["length"] | DEFAULT_PIN_LENGTH;
                
                LOG_INFO("PinManager", "PIN config loaded:");
                LOG_INFO("PinManager", "  Device PIN: " + String(enabledForDevice ? "enabled" : "disabled"));
                LOG_INFO("PinManager", "  BLE PIN: " + String(enabledForBle ? "enabled" : "disabled"));
                LOG_INFO("PinManager", "  PIN length: " + String(currentPinLength));
            } else {
                LOG_ERROR("PinManager", "Failed to parse PIN config: " + String(error.c_str()));
            }
        } else {
            LOG_ERROR("PinManager", "Failed to open PIN config file");
        }
    } else {
        LOG_INFO("PinManager", "PIN config file not found, using defaults");
    }
}

void PinManager::savePinConfig() {
    LOG_DEBUG("PinManager", "Saving PIN configuration");
    JsonDocument doc;
    doc["enabledForDevice"] = enabledForDevice;
    doc["enabledForBle"] = enabledForBle;
    doc["hash"] = pinHash;
    doc["length"] = currentPinLength;
    
    // Сохраняем старый формат для обратной совместимости
    doc["enabled"] = (enabledForDevice || enabledForBle);

    String content;
    size_t jsonSize = serializeJson(doc, content);
    if (jsonSize == 0) {
        LOG_ERROR("PinManager", "Failed to serialize PIN config to JSON");
        return;
    }

    String encryptedContent = CryptoManager::getInstance().encrypt(content);
    if (encryptedContent.isEmpty()) {
        LOG_ERROR("PinManager", "Failed to encrypt PIN config");
        return;
    }

    fs::File configFile = LittleFS.open(PIN_FILE, "w");
    if (configFile) {
        size_t bytesWritten = configFile.print(encryptedContent);
        configFile.close();
        if (bytesWritten > 0) {
            LOG_INFO("PinManager", "PIN config saved successfully:");
            LOG_INFO("PinManager", "  Device PIN: " + String(enabledForDevice ? "enabled" : "disabled"));
            LOG_INFO("PinManager", "  BLE PIN: " + String(enabledForBle ? "enabled" : "disabled"));
        } else {
            LOG_ERROR("PinManager", "Failed to write PIN config data");
        }
    } else {
        LOG_ERROR("PinManager", "Failed to open PIN config file for writing");
    }
}

void PinManager::updatePinScreen(int currentPosition, int currentDigit, const String& enteredPin) {
    TFT_eSPI* tft = displayManager.getTft();
    int centerX = tft->width() / 2;

    // Обновляем только маску PIN-кода
    String pinMask = "";
    for (int i = 0; i < enteredPin.length(); i++) pinMask += "*";
    for (int i = 0; i < (currentPinLength - enteredPin.length()); i++) pinMask += ".";
    
    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(3);
    // Очищаем предыдущую маску и рисуем новую
    tft->fillRect(0, 50, tft->width(), 24, TFT_BLACK); // Adjusted Y for clearing
    tft->drawString(pinMask, centerX, 60); // Adjusted Y for better vertical centering

    // Обновляем только селектор цифр
    String selector = "< " + String(currentDigit) + " >";
    tft->setTextSize(2);
    // Очищаем предыдущий селектор и рисуем новый
    tft->fillRect(0, 85, tft->width(), 16, TFT_BLACK); // Adjusted Y for clearing
    tft->drawString(selector, centerX, 95); // Adjusted Y for better vertical centering
}

void PinManager::drawPinScreen() {
    TFT_eSPI* tft = displayManager.getTft();
    tft->fillScreen(TFT_BLACK);
    tft->setTextDatum(MC_DATUM); // Устанавливаем выравнивание по центру

    int centerX = tft->width() / 2;

    // Рисуем заголовок
    tft->setTextSize(2);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    displayManager.drawUtf8Centered("请输入 PIN 码", centerX, 25, TFT_WHITE, TFT_BLACK, true); // Adjusted Y for better spacing
    
    // Add cancel hint
    tft->setTextSize(1);
    displayManager.drawUtf8Centered("同时按住两个按键可取消", centerX, tft->height() - 10, TFT_WHITE, TFT_BLACK, true);
}

bool PinManager::requestPin() {
    if (!isPinSet()) {
        LOG_INFO("PinManager", "No PIN set, granting access");
        return true; // If PIN is not set, access is granted.
    }
    
    LOG_INFO("PinManager", "PIN entry requested");

    String enteredPin = "";
    int currentDigit = 0;
    unsigned long lastButtonPress = 0;
    const int debounce = 150; // 🔋 Уменьшено для возможности зажимать кнопку
    
    // 🔋 КРИТИЧНО: Снижаем яркость экрана для уменьшения потребления тока
    // Это предотвращает brownout при питании от батареи
    displayManager.setBrightness(128); // 50% яркости вместо 100%
    
    // Убедимся что экран правильно инициализирован перед отрисовкой PIN экрана
    drawPinScreen(); // Начальная отрисовка (один раз)
    updatePinScreen(enteredPin.length(), currentDigit, enteredPin); // Первоначальное отображение маски и селектора

    while (true) {
        // Reset watchdog timer to prevent timeout
        esp_task_wdt_reset();
        
        // Check for cancellation (holding both buttons)
        if (digitalRead(BUTTON_1) == LOW && digitalRead(BUTTON_2) == LOW) {
            unsigned long pressStartTime = millis();
            while(digitalRead(BUTTON_1) == LOW && digitalRead(BUTTON_2) == LOW) {
                esp_task_wdt_reset(); // Reset watchdog during button hold
                if (millis() - pressStartTime > 1000) { // Hold for 1 second to cancel
                    LOG_INFO("PinManager", "PIN entry cancelled by user");
                    
                    // 🔋 Восстанавливаем полную яркость перед выходом
                    displayManager.setBrightness(255);
                    
                    displayManager.init();
                    displayManager.showMessage("已取消", 10, 50, false, 2);
                    delay(1000);
                    return false;
                }
            }
        }

        // Кнопка 1 (пин 35) - переключение цифры
        if (digitalRead(BUTTON_1) == LOW && (millis() - lastButtonPress > debounce)) {
            lastButtonPress = millis();
            currentDigit = (currentDigit + 1) % 10;
            
            updatePinScreen(enteredPin.length(), currentDigit, enteredPin); // Обновляем только изменяемые части
            
            // 🔋 КРИТИЧНО: Ждем отпускания кнопки с задержками для стабилизации
            // Это предотвращает множественные быстрые обновления при зажатии
            unsigned long buttonHoldStart = millis();
            while(digitalRead(BUTTON_1) == LOW) {
                esp_task_wdt_reset();
                delay(20); // Увеличенная задержка при удержании
                
                // Если кнопка зажата больше 300ms - выходим и позволяем следующее срабатывание
                if (millis() - buttonHoldStart > 300) {
                    break;
                }
            }
            
            // Дополнительная задержка после отпускания для стабилизации питания
            delay(30);
            esp_task_wdt_reset();
        }

        // Кнопка 2 (пин 0) - подтверждение цифры
        if (digitalRead(BUTTON_2) == LOW && (millis() - lastButtonPress > debounce)) {
            lastButtonPress = millis();
            enteredPin += String(currentDigit);
            currentDigit = 0;
            
            updatePinScreen(enteredPin.length(), currentDigit, enteredPin); // Обновляем маску
            
            // 🔋 КРИТИЧНО: Ждем отпускания кнопки с задержками для стабилизации
            unsigned long buttonHoldStart = millis();
            while(digitalRead(BUTTON_2) == LOW) {
                esp_task_wdt_reset();
                delay(20);
                
                // Если кнопка зажата больше 300ms - выходим
                if (millis() - buttonHoldStart > 300) {
                    break;
                }
            }
            
            // Дополнительная задержка после отпускания для стабилизации питания
            delay(30);
            esp_task_wdt_reset();

            if (enteredPin.length() >= currentPinLength) {
                TFT_eSPI* tft = displayManager.getTft();
                int centerX = tft->width() / 2;
                tft->setTextDatum(MC_DATUM);

                if (checkPin(enteredPin)) {
                    LOG_INFO("PinManager", "PIN verification successful");
                    tft->fillScreen(TFT_BLACK);
                    tft->setTextSize(3);
                    displayManager.drawUtf8Centered("PIN 正确", centerX, 67, TFT_WHITE, TFT_BLACK, true); // Centered vertically
                    delay(1000);
                    
                    // 🔋 Восстанавливаем полную яркость после успешного ввода
                    displayManager.setBrightness(255);
                    
                    return true;
                } else {
                    LOG_WARNING("PinManager", "PIN verification failed");
                    tft->fillScreen(TFT_BLACK);
                    tft->setTextSize(2);
                    tft->setTextColor(TFT_RED);
                    displayManager.drawUtf8Centered("PIN 错误", centerX, 67, TFT_RED, TFT_BLACK, true); // Centered vertically
                    delay(2000);
                    
                    // 🔋 Восстанавливаем полную яркость перед выходом
                    displayManager.setBrightness(255);
                    
                    return false; // Return false on wrong PIN
                }
            }
        }
        delay(50);
    }
}

void PinManager::setPin(const String& newPin) {
    if (newPin.length() > 0) {
        pinHash = CryptoManager::getInstance().hashPassword(newPin);
    }
}

// New separate methods for device and BLE PIN control
void PinManager::setPinEnabledForDevice(bool enabled) {
    enabledForDevice = enabled;
    LOG_INFO("PinManager", "Device PIN " + String(enabled ? "enabled" : "disabled"));
}

void PinManager::setPinEnabledForBle(bool enabled) {
    enabledForBle = enabled;
    LOG_INFO("PinManager", "BLE PIN " + String(enabled ? "enabled" : "disabled"));
}

bool PinManager::isPinEnabledForDevice() {
    return enabledForDevice;
}

bool PinManager::isPinEnabledForBle() {
    return enabledForBle;
}

// Legacy methods for backward compatibility
void PinManager::setEnabled(bool newStatus) {
    enabledForDevice = newStatus;
    enabledForBle = newStatus;
    LOG_INFO("PinManager", "Legacy PIN setting - both device and BLE " + String(newStatus ? "enabled" : "disabled"));
}

bool PinManager::isPinEnabled() {
    return enabledForDevice || enabledForBle;
}

int PinManager::getPinLength() {
    return currentPinLength;
}

void PinManager::setPinLength(int newLength) {
    if (newLength >= 4 && newLength <= MAX_PIN_LENGTH) {
        currentPinLength = newLength;
        LOG_INFO("PinManager", "PIN length set to: " + String(newLength));
    } else {
        LOG_WARNING("PinManager", "Invalid PIN length: " + String(newLength) + ". Must be 4-" + String(MAX_PIN_LENGTH));
    }
}

void PinManager::saveConfig() {
    savePinConfig();
}

bool PinManager::isPinSet() {
    return pinHash.length() > 0;
}

bool PinManager::checkPin(const String& pin) {
    return CryptoManager::getInstance().verifyPassword(pin, pinHash);
}


