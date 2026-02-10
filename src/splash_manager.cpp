#include "splash_manager.h"
#include "LittleFS.h"
#include "embedded_splashes.h"
#include "log_manager.h"
#include <ArduinoJson.h>
#include <FS.h>

SplashScreenManager::SplashScreenManager(DisplayManager& displayManager) 
    : _displayManager(displayManager) {}

void SplashScreenManager::displayEmbeddedSplash(const String& mode) {
    const unsigned char* splashData = nullptr;
    unsigned int splashLen = 0;
    String splashName = "";
    
    // 🎨 Модульный выбор splash screen
    if (mode == "bladerunner") {
        splashData = splash_bladerunner;
        splashLen = splash_bladerunner_len;
        splashName = "BladeRunner";
    } else if (mode == "combs") {
        splashData = splash_combs;
        splashLen = splash_combs_len;
        splashName = "Combs";
    } else if (mode == "securegen") {
        splashData = splash_securegen;
        splashLen = splash_securegen_len;
        splashName = "SecureGen";
    } else if (mode == "disabled") {
        LOG_INFO("SplashManager", "❌ Splash screen disabled by config");
        return; // Не показываем splash
    } else {
        // Неизвестный режим - отключаем splash
        LOG_WARNING("SplashManager", "⚠️ Unknown splash mode: " + mode + ", disabling splash");
        return;
    }
    
    if (!splashData || splashLen == 0) {
        LOG_ERROR("SplashManager", "❌ Invalid embedded splash data");
        return;
    }
    
    LOG_INFO("SplashManager", "🖼️ Displaying embedded splash: " + splashName);
    
    // Копируем из PROGMEM в RAM
    uint16_t* imageBuffer = (uint16_t*) malloc(splashLen);
    if (imageBuffer) {
        // 🌌 ШАГ 1: Отобразить изображение (экран уже чёрный, яркость 0 из initForSplash)
        memcpy_P(imageBuffer, splashData, splashLen);
        _displayManager.getTft()->pushImage(0, 0, 
            SPLASH_IMAGE_WIDTH, SPLASH_IMAGE_HEIGHT, imageBuffer);
        free(imageBuffer);
        
        // Небольшая пауза для завершения отрисовки
        delay(100);
        
        // 🌌 ШАГ 3: Fade in - плавное появление (0.5s: 0→255)
        const int fadeInDuration = 500; // ms
        const int fadeInSteps = 50;
        const int fadeInDelay = fadeInDuration / fadeInSteps;
        for (int i = 0; i <= fadeInSteps; i++) {
            uint8_t brightness = (255 * i) / fadeInSteps;
            _displayManager.setBrightness(brightness);
            delay(fadeInDelay);
        }
        
        // 🌌 ШАГ 4: Hold - показ при полной яркости (1.0s)
        delay(1000);
        
        // 🌌 ШАГ 5: Fade out - плавное затухание (0.5s: 255⁂0)
        const int fadeOutDuration = 500; // ms
        const int fadeOutSteps = 50;
        const int fadeOutDelay = fadeOutDuration / fadeOutSteps;
        for (int i = fadeOutSteps; i >= 0; i--) {
            uint8_t brightness = (255 * i) / fadeOutSteps;
            _displayManager.setBrightness(brightness);
            delay(fadeOutDelay);
        }
        
        // 🌌 ШАГ 6: Восстановление полной яркости для основного экрана
        _displayManager.turnOn();
    } else {
        LOG_ERROR("SplashManager", "❌ Failed to allocate memory for splash");
    }
}

// displayCustomSplash() REMOVED - custom splash feature disabled for security

void SplashScreenManager::displaySplashScreen() {
    // Custom splash upload disabled for security - only embedded splash screens
    String mode = loadSplashConfig();
    displayEmbeddedSplash(mode);
}

// deleteSplashImage() REMOVED - custom splash feature disabled for security

String SplashScreenManager::loadSplashConfig() {
    if (!LittleFS.exists(SPLASH_CONFIG_PATH)) {
        LOG_DEBUG("SplashManager", "🔍 Splash config not found, using default (disabled)");
        return "disabled"; // По умолчанию = выключено (для factory reset)
    }
    
    fs::File configFile = LittleFS.open(SPLASH_CONFIG_PATH, "r");
    if (!configFile) {
        LOG_ERROR("SplashManager", "❌ Failed to open splash config");
        return "disabled";
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        LOG_ERROR("SplashManager", "❌ Failed to parse splash config JSON");
        return "disabled";
    }
    
    String mode = doc["mode"] | "disabled";
    LOG_INFO("SplashManager", "📋 Loaded splash config: " + mode);
    return mode;
}

bool SplashScreenManager::saveSplashConfig(const String& mode) {
    // Валидация
    if (mode != "disabled" && mode != "bladerunner" && mode != "combs" && mode != "securegen") {
        LOG_ERROR("SplashManager", "❌ Invalid splash mode: " + mode);
        return false;
    }
    
    JsonDocument doc;
    doc["mode"] = mode;
    
    fs::File configFile = LittleFS.open(SPLASH_CONFIG_PATH, "w");
    if (!configFile) {
        LOG_ERROR("SplashManager", "❌ Failed to open splash config for writing");
        return false;
    }
    
    size_t bytesWritten = serializeJson(doc, configFile);
    configFile.close();
    
    if (bytesWritten > 0) {
        LOG_INFO("SplashManager", "✅ Saved splash config: " + mode);
        return true;
    }
    
    LOG_ERROR("SplashManager", "❌ Failed to write splash config");
    return false;
}