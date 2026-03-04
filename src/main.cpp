#include <Arduino.h>
#include "config.h"
#include "battery_manager.h"
#include "display_manager.h"
#include "log_manager.h"
#include <esp_bt.h>
#include <nvs_flash.h>
#include <ESPmDNS.h>
#include "web_server.h"
#include "totp_generator.h"
#include "LittleFS.h"
#include "esp_sleep.h"
#include "splash_manager.h"
#include "pin_manager.h"
#include "battery_manager.h"
#include "config_manager.h"
#include "crypto_manager.h"
#include "PasswordManager.h"
#include "ble_keyboard_manager.h"
#include <esp_gap_ble_api.h>
#include "log_manager.h"
#include "web_admin_manager.h"
#include "app_modes.h" // Используем новый общий заголовок
#include <esp_task_wdt.h>

#ifdef SECURE_LAYER_ENABLED
#include "secure_layer_manager.h"
#include "traffic_obfuscation_manager.h"
#include "url_obfuscation_manager.h"
#endif

// Global flag for device restart
bool shouldRestart = false;

#define WDT_TIMEOUT 5

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Глобальные объекты менеджеров
DisplayManager displayManager;
ConfigManager configManager;
KeyManager keyManager;
PasswordManager passwordManager;
SplashScreenManager splashManager(displayManager);
PinManager pinManager(displayManager);
BatteryManager batteryManager(34, 14);
BleKeyboardManager bleKeyboardManager(DEFAULT_BLE_DEVICE_NAME, "Lord", 100);
WifiManager wifiManager(displayManager, configManager); 
TOTPGenerator totpGenerator;
WebServerManager webServerManager(keyManager, splashManager, displayManager, pinManager, configManager, passwordManager, totpGenerator);

#ifdef SECURE_LAYER_ENABLED
SecureLayerManager& secureLayerManager = SecureLayerManager::getInstance();
TrafficObfuscationManager& trafficObfuscationManager = TrafficObfuscationManager::getInstance();
#endif

// Глобальные переменные состояния
static AppMode currentMode = AppMode::TOTP;
static int currentKeyIndex = 0;
static int currentPasswordIndex = 0;
static int previousKeyIndex = -1;
static int previousPasswordIndex = -1;
unsigned long lastButtonPressTime = 0; 
const int debounceDelay = 300; 
const int factoryResetHoldTime = 5000;
const int powerOffHoldTime = 5000;
unsigned long lastActivityTime = 0;
bool isScreenOn = true;

unsigned long bothButtonsPressStartTime = 0;
bool bleActionTriggered = false;

unsigned long lastBatteryCheckTime = 0;
const int batteryCheckInterval = 1000;
static int lastBatteryPercentage = -1;


unsigned long lastTotpUpdateTime = 0;
const int totpUpdateInterval = 250;

void wakeDisplaySafely(const char* reason);

void showWebServerInfoPage() {
    // 🔄 Не вызываем init() - избегаем мигания!
    TFT_eSPI* tft = displayManager.getTft();
    const ThemeColors* colors = displayManager.getCurrentThemeColors();
    
    // 🌌 Плавное затухание перед отрисовкой
    for (int i = 255; i >= 0; i -= 15) {
        displayManager.setBrightness(i);
        delay(10);
    }
    
    // Отрисовка при погашенном экране (не видна пользователю)
    tft->fillScreen(colors->background_dark);
    tft->setTextDatum(MC_DATUM); // Middle Center alignment
    
    // Title
    tft->setTextColor(colors->accent_primary, colors->background_dark);
    tft->setTextSize(2);
    displayManager.drawUtf8Centered("Web 管理已启动", tft->width() / 2, 24, colors->accent_primary, colors->background_dark, true);
    
    // IP Address
    String ip = wifiManager.getIP();
    tft->setTextColor(colors->text_primary, colors->background_dark);
    tft->setTextSize(2);
    tft->drawString(ip, tft->width() / 2, 60);
    
    // Domain - защита от краша
    tft->setTextColor(colors->accent_secondary, colors->background_dark);
    tft->setTextSize(1);
    String mdnsHostname = configManager.loadMdnsHostname();
    mdnsHostname += ".local";
    tft->drawString(mdnsHostname, tft->width() / 2, 85);
    
    // Instructions
    tft->setTextColor(colors->text_secondary, colors->background_dark);
    tft->setTextSize(1);
    displayManager.drawUtf8Centered("请在浏览器访问上方地址", tft->width() / 2, 105, colors->text_secondary, colors->background_dark, true);
    
    // 🌌 Плавное появление
    for (int i = 0; i <= 255; i += 15) {
        displayManager.setBrightness(i);
        delay(10);
    }
    displayManager.turnOn(); // Полная яркость
    
    delay(3000);
    
    // 🧹 КРИТИЧНО: Очистка экрана перед возвратом к TOTP
    // Без этого текст "可开始连接" остается под шкалой!
    tft->fillScreen(colors->background_dark);
}

void handleFactoryResetOnBoot() {
    displayManager.init();
    displayManager.showMessage("同时按住两个按键", 10, 20, false, 2);
    displayManager.showMessage("可恢复出厂设置", 10, 40, false, 2);
    
    unsigned long startTime = millis();
    
    while(digitalRead(BUTTON_1) == LOW && digitalRead(BUTTON_2) == LOW) {
        unsigned long holdTime = millis() - startTime;
        
        if (holdTime > factoryResetHoldTime) {
            displayManager.init();
            displayManager.showMessage("恢复出厂中!", 10, 30, true, 2);
            
            LOG_CRITICAL("Main", "--- FACTORY RESET ---");
            LOG_INFO("Main", "Clearing active web sessions...");
            webServerManager.clearSession();
            LOG_INFO("Main", "Deleting files...");
            LittleFS.remove(KEYS_FILE);
            LittleFS.remove("/wifi_config.json");
            // SPLASH_IMAGE_PATH removed - custom splash upload disabled for security
            LittleFS.remove("/splash_config.json");  // Splash mode config (reset to disabled)
            LittleFS.remove(PIN_FILE);
            LittleFS.remove(CONFIG_FILE); // Resets display timeout to default (30s) + AP password to "12345678"
            LittleFS.remove(DEVICE_KEY_FILE);
            LittleFS.remove(PASSWORD_FILE);
            LittleFS.remove(BLE_CONFIG_FILE);
            LittleFS.remove(WEB_ADMIN_FILE);
            LittleFS.remove(MDNS_CONFIG_FILE); // <-- СБРОС MDNS
            LittleFS.remove(LOGIN_STATE_FILE); // <-- СБРОС СОСТОЯНИЯ ЛОГИНА
            LittleFS.remove("/ble_pin.json.enc"); // <-- СБРОС BLE PIN
            LittleFS.remove("/session.json.enc"); // <-- СБРОС СЕССИЙ И CSRF
            
            // 🔗 URL Obfuscation: Удаление boot counter и всех mappings
            LOG_INFO("Main", "Clearing URL obfuscation data...");
            LittleFS.remove("/boot_counter.txt"); // <-- СБРОС BOOT COUNTER
            
            // 🗑️ Удаляем все url_mappings_*.json файлы
            fs::File root = LittleFS.open("/", "r");
            if (root) {
                fs::File file = root.openNextFile();
                while (file) {
                    String filename = String(file.name());
                    if (filename.startsWith("/url_mappings_") && filename.endsWith(".json")) {
                        LOG_DEBUG("Main", "Removing URL mapping file: " + filename);
                        LittleFS.remove(filename);
                    }
                    file = root.openNextFile();
                }
            }
            LOG_INFO("Main", "URL obfuscation data cleared");
            
            LOG_INFO("Main", "File deletion complete");
            
            // КРИТИЧНО: Очистка BLE bonding ключей через NVS partition erase
            LOG_INFO("Main", "BLE bonding keys will be cleared by NVS partition erase");
            
            // Дополнительная очистка NVS BLE раздела
            nvs_flash_erase_partition("nvs");
            LOG_INFO("Main", "NVS partition cleared");

            displayManager.showMessage("完成，正在重启...", 10, 60);
            
            delay(2500);
            ESP.restart();
        }
        
        int progress = (holdTime * 100) / factoryResetHoldTime;
        displayManager.showMessage("正在重置: " + String(progress) + "%", 10, 100);
        delay(100);
    }
    LOG_INFO("Main", "Factory reset aborted. Continuing boot");
    displayManager.init();
}

void setup() {
    Serial.begin(115200);
    LogManager::getInstance().begin();
    LOG_INFO("Main", "T-Disp-TOTP Booting Up");

    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);

    if (digitalRead(BUTTON_1) == LOW && digitalRead(BUTTON_2) == LOW) {
        if (LittleFS.begin(true)) {
            Theme savedTheme = configManager.loadTheme();
            displayManager.setTheme(savedTheme);
            handleFactoryResetOnBoot();
        } else {
            DisplayManager tempDisplay;
            tempDisplay.init();
            tempDisplay.showMessage("文件系统挂载失败!", 10, 30, true);
            while(1);
        }
    }

    LOG_INFO("Main", "Initializing Battery Manager...");
    batteryManager.begin();
    LOG_INFO("Main", "Initializing LittleFS...");
    if (!LittleFS.begin(true)) {
        LOG_CRITICAL("Main", "LittleFS Mount Failed!");
        DisplayManager tempDisplay;
        tempDisplay.init();
        tempDisplay.showMessage("文件系统挂载失败", 10, 30, true);
        while(1);
    }

    LOG_INFO("Main", "Initializing Crypto Manager...");
    CryptoManager::getInstance().begin();

#ifdef SECURE_LAYER_ENABLED
    LOG_INFO("Main", "Initializing Secure Layer Manager...");
    if (secureLayerManager.begin()) {
        LOG_INFO("Main", "Secure Layer Manager initialized successfully");
    } else {
        LOG_ERROR("Main", "Failed to initialize Secure Layer Manager");
    }
    
    // ❌ MOVED: TrafficObfuscation инициализируется в WebServerManager::start()
    // Не должен работать если веб-сервер не запущен!
#endif
    
    LOG_INFO("Main", "Initializing Web Admin Manager...");
    WebAdminManager::getInstance().begin();

    LOG_INFO("Main", "Loading theme...");
    Theme savedTheme = configManager.loadTheme();
    displayManager.setTheme(savedTheme);
    
    LOG_INFO("Main", "Loading BLE device name...");
    String savedBleDeviceName = configManager.loadBleDeviceName();
    bleKeyboardManager.setDeviceName(savedBleDeviceName);
    
    LOG_INFO("Main", "Setting up BLE display manager...");
    bleKeyboardManager.setDisplayManager(&displayManager);
    
    LOG_INFO("Main", "Setting up web server BLE reference...");
    webServerManager.setBleKeyboardManager(&bleKeyboardManager);
    webServerManager.setWifiManager(&wifiManager);
    
    String startupMode = configManager.getStartupMode();
    LOG_INFO("Main", "Loaded startup mode: " + startupMode);
    if (startupMode == "password") {
        currentMode = AppMode::PASSWORD;
        LOG_INFO("Main", "Starting in Password Manager mode");
    } else {
        currentMode = AppMode::TOTP;
        LOG_INFO("Main", "Starting in TOTP Authenticator mode");
    }

    LOG_INFO("Main", "Initializing Display, Key, Password, and Pin Managers...");
    // Ранняя инициализация для splash (без заполнения экрана и без включения яркости)
    displayManager.initForSplash();
    keyManager.begin();
    passwordManager.begin();
    pinManager.begin();
    
    LOG_INFO("Main", "Displaying splash screen...");
    splashManager.displaySplashScreen();
    
    // 🔧 Полная инициализация display после splash (перед PIN проверкой)
    displayManager.init();
    
    LOG_INFO("Main", "Checking device startup PIN...");
    if (pinManager.isPinEnabledForDevice() && pinManager.isPinSet()) {
        LOG_INFO("Main", "Device PIN enabled, requesting PIN...");
        while (!pinManager.requestPin()) {
            LOG_WARNING("Main", "PIN access denied. Retrying...");
            delay(1000);
        }
        LOG_INFO("Main", "Device PIN access granted. Continuing startup...");
        // Переинициализируем дисплей после PIN проверки
        displayManager.init();
    } else {
            LOG_INFO("Main", "Device PIN disabled or not set. Continuing startup...");
    }
    
    displayManager.updateMessage("初始化中...", 10, 10, 2);
    
    // 🌌 ПРОМПТИНГ ВЫБОРА РЕЖИМА (AP/Offline/WiFi)
    LOG_INFO("Main", "Prompting for startup mode...");
    StartupMode selectedMode = displayManager.promptModeSelection();
    
    // Переменная для отслеживания синхронизации времени
    struct tm timeinfo;
    bool timeSynced = false;
    
    if (selectedMode == StartupMode::AP_MODE) {
        // 📡 AP MODE
        LOG_INFO("Main", "User chose AP Mode. Starting Access Point...");
        
        // Генерация имени AP на основе MAC
        String apName = "ESP32-TOTP-" + String(
            WiFi.macAddress().substring(12, 14) + 
            WiFi.macAddress().substring(15, 17)
        );
        String apPassword = configManager.loadApPassword();
        
        // Запуск AP точки
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apName.c_str(), apPassword.c_str());
        
        // Запуск mDNS для AP режима
        String hostname = configManager.loadMdnsHostname();
        if (MDNS.begin(hostname.c_str())) {
            LOG_INFO("Main", "mDNS started in AP mode. Access via: http://" + hostname + ".local");
            MDNS.addService("http", "tcp", 80);
        } else {
            LOG_ERROR("Main", "Failed to start mDNS in AP mode");
        }
        
        // Отображение информации на экране
        displayManager.init();
        displayManager.showMessage("AP 模式已启用", 10, 20, false, 2);
        displayManager.showMessage("网络: " + apName, 10, 40, false, 1);
        displayManager.showMessage("密码: " + apPassword, 10, 55, false, 1);
        displayManager.showMessage("IP: " + WiFi.softAPIP().toString(), 10, 70, false, 1);
        displayManager.showMessage("域名: " + hostname + ".local", 10, 85, false, 1);
        displayManager.showMessage("请连接到网络", 10, 100, false, 1);
        displayManager.showMessage("以访问 Web 界面", 10, 115, false, 1);
        
        // Автозапуск веб-сервера в AP режиме
        delay(3000);
        LOG_INFO("Main", "Auto-starting Web Server in AP Mode...");
        webServerManager.start();
        
        // Очистка экрана
        displayManager.clearMessageArea(0, 0, 240, 135);
        
        // ❗ ПРОПУСКАЕМ WiFi подключение и синхронизацию времени
        // TOTP коды будут показывать "TIME 未同步"
        timeSynced = false;
        
    } else if (selectedMode == StartupMode::OFFLINE_MODE) {
        // 🔌 OFFLINE MODE
        LOG_INFO("Main", "User chose 离线模式. No WiFi, no AP, no web server.");
        
        // Полное отключение WiFi
        WiFi.mode(WIFI_OFF);
        
        // Отображение информации
        displayManager.init();
        displayManager.showMessage("离线模式", 10, 20, false, 2);
        displayManager.showMessage("无 WiFi 连接", 10, 40, false, 1);
        displayManager.showMessage("BLE 与密码功能可用", 10, 55, false, 1);
        displayManager.showMessage("TOTP：未同步", 10, 70, false, 1);
        delay(3000);
        
        // Очистка экрана
        displayManager.clearMessageArea(0, 0, 240, 135);
        
        // ❗ ПРОПУСКАЕМ: WiFi, веб-сервер, синхронизацию времени
        // Работают только: TOTP (несинхронизированный), пароли, BLE
        timeSynced = false;
        
    } else {
        // 🌐 WIFI MODE (по умолчанию)
        LOG_INFO("Main", "User chose WiFi mode (or timeout). Connecting to WiFi...");
        displayManager.updateMessage("正在连接 WiFi...", 10, 10, 2);
        
        if (!wifiManager.connect()) {
            LOG_WARNING("Main", "No WiFi credentials found. Starting config portal...");
            
            // 🚫 Отключаем watchdog перед Config Portal (иначе async_tcp вызывает timeout)
            esp_task_wdt_deinit();
            LOG_INFO("Main", "Watchdog disabled for Config Portal mode");
            
            wifiManager.startConfigPortal();
            webServerManager.startConfigServer();
            
            // ⚠️ Config portal - это setup режим, который может работать долго
            LOG_INFO("Main", "Config portal active. Connect to ESP32-TOTP-Setup to configure WiFi.");
            LOG_INFO("Main", "Note: Keeping system alive for async web server.");
            
            // Бесконечный цикл для Config Portal
            while(1) {
                yield(); // Позволяем async tasks работать
                delay(100); // Короткая задержка для экономии CPU
                vTaskDelay(1); // Дополнительный yield для FreeRTOS
            }
        }
        LOG_INFO("Main", "WiFi Connected! IP: " + wifiManager.getIP());

        // 🕗 Time Sync (только для WiFi Mode)
        // 🌍 Используем разные NTP сервера для повышения надежности
        const char* ntpServers[] = {
            "pool.ntp.org",        // Global NTP pool
            "time.google.com",     // Google NTP (fast & reliable)
            "time.cloudflare.com"  // Cloudflare NTP (1.1.1.1)
        };
        
        LOG_INFO("Main", "Syncing time with multiple NTP servers...");
        for (int i = 0; i < 3; i++) {
            // 🔄 Обновляем только текст без полной перерисовки
            displayManager.updateMessage("正在同步时间... (" + String(i + 1) + "/3)", 10, 10, 2);
            
            // ✅ Каждая попытка использует СВОЙ NTP сервер
            LOG_INFO("Main", "NTP attempt " + String(i+1) + ": " + String(ntpServers[i]));
            configTime(0, 0, ntpServers[i]);
            
            // Даем время на отправку и обработку NTP запроса
            delay(800); // Увеличено с 500ms для стабильности
            
            if (getLocalTime(&timeinfo, 5000)) {
                timeSynced = true;
                LOG_INFO("Main", "Time Synced Successfully on attempt " + String(i+1) + " (" + String(ntpServers[i]) + ")!");
                // 🔄 Обновляем только текст
                displayManager.updateMessage("时间同步完成！", 10, 10, 2);
                delay(1000);
                break;
            }
            
            LOG_WARNING("Main", "NTP server " + String(ntpServers[i]) + " failed");
            
            // ⌨️ Задержка перед следующей попыткой (кроме последней)
            if (i < 2) {
                delay(1000); // 1 секунда между попытками
            }
        }

        if (!timeSynced) {
            // ⚠️ OFFLINE FALLBACK: Продолжаем работу без синхронизации времени
            // TOTP будет показывать "未同步", но пароли и BLE работают нормально
            LOG_WARNING("Main", "All 3 NTP servers failed (pool.ntp.org, time.google.com, time.cloudflare.com)");
            LOG_WARNING("Main", "Continuing in offline mode. TOTP：未同步, 密码功能：正常");
            
            displayManager.init();
            displayManager.showMessage("警告:", 10, 20, false, 2);
            displayManager.showMessage("时间同步失败!", 10, 40, false, 2);
            displayManager.showMessage("TOTP：未同步", 10, 60, false, 1);
            displayManager.showMessage("密码功能：正常", 10, 75, false, 1);
            displayManager.showMessage("继续运行...", 10, 95, false, 1);
            delay(3000);
            
            // Устанавливаем timeSynced = false для offline режима
            timeSynced = false;
        }

        // Отключаем WiFi для экономии батареи (независимо от статуса синхронизации)
        if (timeSynced) {
            LOG_INFO("Main", "Time synced successfully. Disconnecting WiFi to save power.");
        } else {
            LOG_INFO("Main", "Disconnecting WiFi to save power (time not synced).");
        }
        wifiManager.disconnect();

        // Check if web server should auto-start
        bool autoStartWebServer = configManager.getWebServerAutoStart();
        if (autoStartWebServer) {
            LOG_INFO("Main", "Auto-starting Web Server (flag was set)...");
            // Reset the flag immediately to prevent auto-start on subsequent boots
            configManager.setWebServerAutoStart(false);
            
            if (wifiManager.connectSilent()) {
                LOG_INFO("Main", "WiFi Reconnected. Starting Web Server.");
                webServerManager.start();
                delay(500); // Даём время async web server полностью инициализироваться
                showWebServerInfoPage();
            } else {
                LOG_ERROR("Main", "WiFi reconnection failed! Web server not started.");
                displayManager.init();
                displayManager.showMessage("错误:", 10, 20, true, 2);
                displayManager.showMessage("WiFi 重连失败!", 10, 40, false, 2);
                delay(2000);
            }
        } else {
            LOG_INFO("Main", "Prompting for Web Server...");
            if (displayManager.promptWebServerSelection()) {
                LOG_INFO("Main", "User chose to start Web Server. Reconnecting to WiFi...");
                if (wifiManager.connectSilent()) {
                    LOG_INFO("Main", "WiFi Reconnected. Starting Web Server.");
                    webServerManager.start();
                    delay(500); // Даём время async web server полностью инициализироваться
                    showWebServerInfoPage();
                } else {
                    LOG_ERROR("Main", "WiFi reconnection failed! Web server not started.");
                    displayManager.init();
                    displayManager.showMessage("错误:", 10, 20, true, 2);
                    displayManager.showMessage("WiFi 重连失败!", 10, 40, false, 2);
                    delay(2000);
                }
            } else {
                LOG_INFO("Main", "User chose not to start Web Server.");
                // ✅ Промптинг уже очистил экран перед return
            }
        }
    } // Конец WiFi Mode
    
    // ✅ displayManager.init() уже вызван - очищаем область сообщений перед входом в основной цикл
    displayManager.clearMessageArea(0, 0, 240, 60);

    LOG_INFO("Main", "Main Loop Started");
    lastActivityTime = millis();

    LOG_INFO("Main", "Initializing Watchdog Timer...");
    if (esp_task_wdt_init(WDT_TIMEOUT, true) == ESP_OK) {
        if (esp_task_wdt_add(NULL) == ESP_OK) {
            LOG_INFO("Main", "Watchdog Timer initialized successfully");
        } else {
            LOG_ERROR("Main", "Failed to add task to Watchdog Timer");
        }
    } else {
        LOG_ERROR("Main", "Failed to initialize Watchdog Timer");
    }
}



void handleButtons() {
    static unsigned long button1PressStartTime = 0;
    static unsigned long button2PressStartTime = 0;
    bool buttonPressed = false;

    bool button1_is_pressed = (digitalRead(BUTTON_1) == LOW);
    bool button2_is_pressed = (digitalRead(BUTTON_2) == LOW);

    // --- Логика двойного нажатия (высший приоритет) ---
    if (button1_is_pressed && button2_is_pressed) {
        // Если зажаты обе кнопки, сбрасываем таймеры одиночных нажатий, чтобы предотвратить конфликт
        button1PressStartTime = 0;
        button2PressStartTime = 0;

        // Действие по двойному нажатию валидно только в режиме паролей
        if (currentMode == AppMode::PASSWORD && !passwordManager.getAllPasswords().empty()) {
            if (bothButtonsPressStartTime == 0) {
                bothButtonsPressStartTime = millis();
            }

            unsigned long holdTime = millis() - bothButtonsPressStartTime;

            if (holdTime >= 500) { // Показываем лоадер через 500мс
                if (holdTime >= 2000) { // Если продержали 2с
                    if (!bleActionTriggered) {
                        bleActionTriggered = true;
                        currentMode = AppMode::BLE_ADVERTISING;
                        previousPasswordIndex = -1; 
                        LOG_INFO("Main", "Both buttons held. Switching to BLE_ADVERTISING mode");
                    }
                } else {
                    // Рисуем лоадер для BLE
                    int progress = map(holdTime - 500, 0, 1500, 0, 100);
                    displayManager.drawBleInitLoader(progress);
                }
            }
        }
        // В любом случае выходим, чтобы не обрабатывать одиночные нажатия
        return;
    } else {
        // Если кнопки не зажаты вместе, сбрасываем таймер двойного нажатия
        if (bothButtonsPressStartTime > 0) {
            bothButtonsPressStartTime = 0;
            bleActionTriggered = false;
            previousPasswordIndex = -1;
            displayManager.hideLoader();
        }
    }

    // --- Логика одиночных нажатий (выполняется, только если не зажаты обе кнопки) ---

    // --- Кнопка 1 (GPIO 35) ---
    if (button1_is_pressed) {
        if (button1PressStartTime == 0) {
            button1PressStartTime = millis();
        } else if (millis() - button1PressStartTime > powerOffHoldTime) {
            // Длительное нажатие: переключить режим
            LOG_INFO("Main", "Button 1 LONG PRESS: Switching modes...");
            if (currentMode == AppMode::BLE_ADVERTISING || currentMode == AppMode::BLE_PIN_ENTRY || currentMode == AppMode::BLE_CONFIRM_SEND) {
                bleKeyboardManager.end();
                bleActionTriggered = false;
            }
            currentMode = (currentMode == AppMode::TOTP) ? AppMode::PASSWORD : AppMode::TOTP;
            LOG_INFO("Main", currentMode == AppMode::TOTP ? "Switched to TOTP mode" : "Switched to PASSWORD mode");
            button1PressStartTime = 0;
            buttonPressed = true;
            previousKeyIndex = -1;
            previousPasswordIndex = -1;
            displayManager.hideLoader();
        } else {
            unsigned long holdTime = millis() - button1PressStartTime;
            if (holdTime >= 1000 && holdTime < powerOffHoldTime) {
                int progress = map(holdTime - 1000, 0, 4000, 0, 100);
                String loaderText = (currentMode == AppMode::TOTP) ? "Passwords..." : "TOTP...";
                displayManager.drawGenericLoader(progress, loaderText);
            }
        }
    } else {
        if (button1PressStartTime > 0) {
            displayManager.hideLoader();
            if (millis() - button1PressStartTime < powerOffHoldTime) {
                LOG_DEBUG("Main", "Button 1 SHORT PRESS: Previous item");
                if (currentMode == AppMode::TOTP) {
                    auto keys = keyManager.getAllKeys();
                    if (!keys.empty()) {
                        currentKeyIndex = (currentKeyIndex == 0) ? keys.size() - 1 : currentKeyIndex - 1;
                        displayManager.setKeySwitched(true); // <-- ADDED
                        buttonPressed = true;
                    }
                } else if (currentMode == AppMode::PASSWORD) { 
                    auto passwords = passwordManager.getAllPasswords();
                    if (!passwords.empty()) {
                        currentPasswordIndex = (currentPasswordIndex == 0) ? passwords.size() - 1 : currentPasswordIndex - 1;
                        buttonPressed = true;
                    }
                }
            }
            button1PressStartTime = 0;
        }
    }

    // --- Кнопка 2 (GPIO 0) ---
    if (button2_is_pressed) {
        if (button2PressStartTime == 0) {
            button2PressStartTime = millis();
        } else if (millis() - button2PressStartTime > powerOffHoldTime) {
            LOG_INFO("Main", "Button 2 LONG PRESS: 正在关闭...");
            displayManager.init();
            displayManager.showMessage("正在关闭...", 10, 30, false, 2);
            delay(1000);
            displayManager.turnOff();
            esp_deep_sleep_start();
        } else {
            unsigned long holdTime = millis() - button2PressStartTime;
            if (holdTime >= 1000 && holdTime < powerOffHoldTime) {
                int progress = map(holdTime - 1000, 0, 4000, 0, 100);
                displayManager.drawGenericLoader(progress, "正在关闭...");
            }
        }
    } else {
        if (button2PressStartTime > 0) {
            displayManager.hideLoader();
            if (millis() - button2PressStartTime < powerOffHoldTime) {
                LOG_DEBUG("Main", "Button 2 SHORT PRESS: Next item");
                if (currentMode == AppMode::TOTP) {
                    auto keys = keyManager.getAllKeys();
                    if (!keys.empty()) {
                        currentKeyIndex = (currentKeyIndex + 1) % keys.size();
                        displayManager.setKeySwitched(true); // <-- ADDED
                        buttonPressed = true;
                    }
                } else if (currentMode == AppMode::PASSWORD) {
                    auto passwords = passwordManager.getAllPasswords();
                    if (!passwords.empty()) {
                        currentPasswordIndex = (currentPasswordIndex + 1) % passwords.size();
                        buttonPressed = true;
                    }
                }
            }
            button2PressStartTime = 0;
        }
    }

    if (buttonPressed) {
        lastActivityTime = millis();
        displayManager.hideLoader();
        if (!isScreenOn) {
            LOG_DEBUG("Main", "Button press woke up screen");
            wakeDisplaySafely("handleButtons");
        }
        previousKeyIndex = -1; 
        previousPasswordIndex = -1;
    }
}

// Функция для проверки нажатия кнопок и включения экрана


// 🔋 低电量时平滑唤醒屏幕，避免背光瞬时电流导致 brownout 复位
void wakeDisplaySafely(const char* reason) {
    uint32_t batteryMv = batteryManager.getVoltageMv();

    // 约 55% 以下电量，限制最大亮度并采用渐进点亮
    const bool weakBattery = batteryMv < 3600;
    const uint8_t targetBrightness = weakBattery ? 170 : 255;
    const uint8_t startBrightness = weakBattery ? 32 : 96;

    LOG_INFO("Power", String("Display wake [") + reason + "] battery=" + String((float)batteryMv / 1000.0f, 2) +
                     "V, weak=" + String(weakBattery ? "yes" : "no") +
                     ", targetBrightness=" + String(targetBrightness));

    for (int b = startBrightness; b <= targetBrightness; b += 16) {
        displayManager.setBrightness(static_cast<uint8_t>(b));
        esp_task_wdt_reset();
        delay(6);

        if (b >= static_cast<int>(targetBrightness) - 8) {
            break;
        }
    }

    displayManager.setBrightness(targetBrightness);
    isScreenOn = true;
}
void checkScreenWakeup() {
    static bool button1PreviousState = HIGH;
    static bool button2PreviousState = HIGH;
    
    bool button1Current = digitalRead(BUTTON_1);
    bool button2Current = digitalRead(BUTTON_2);
    
    // Проверяем нажатие любой кнопки (переход от HIGH к LOW)
    if ((button1PreviousState == HIGH && button1Current == LOW) ||
        (button2PreviousState == HIGH && button2Current == LOW)) {
        
        lastActivityTime = millis();
        if (!isScreenOn) {
            LOG_DEBUG("Main", "Button press woke up screen in BLE mode");
            wakeDisplaySafely("checkScreenWakeup");
        }
    }
    
    button1PreviousState = button1Current;
    button2PreviousState = button2Current;
}

void loop() {
    // Сброс Watchdog Timer в начале каждого цикла
    if (esp_task_wdt_reset() != ESP_OK) {
        LOG_ERROR("Main", "Failed to reset Watchdog Timer");
    }
    displayManager.update(); // Обновляем анимации в любом режиме
    
    // Всегда проверяем включение экрана от кнопок
    checkScreenWakeup();
    
    // Проверяем таймаут API веб-сервера и самого сервера
    if (webServerManager.isRunning()) {
        WebAdminManager::getInstance().checkApiTimeout();
        webServerManager.update();
        
#ifdef SECURE_LAYER_ENABLED
        // ❌ DISABLED: Cleanup causes race condition without mutex
        // secureLayerManager.update();
        // ✅ ENABLED: Traffic Obfuscation for decoy traffic generation
        trafficObfuscationManager.update();
        // ✅ ENABLED: URL Obfuscation automatic rotation (daily)
        URLObfuscationManager::getInstance().update();
#endif
    }

    // Handle buttons based on the current mode for BLE states
    if (currentMode != AppMode::BLE_ADVERTISING && currentMode != AppMode::BLE_PIN_ENTRY && currentMode != AppMode::BLE_CONFIRM_SEND) {
        handleButtons();
    }

    // Проверяем таймаут экрана ТОЛЬКО если веб-сервер НЕ активен
    // Когда веб-сервер работает, таймаут экрана полностью игнорируется для предотвращения замедлений
    uint16_t screenTimeoutSeconds = configManager.getDisplayTimeout();
    
    // ВАЖНО: Когда веб-сервер активен, сбрасываем lastActivityTime чтобы при его выключении
    // был нормальный отсчет таймаута, а не моментальный переход в sleep
    if (webServerManager.isRunning()) {
        lastActivityTime = millis();
    }
    
    if (!webServerManager.isRunning() && screenTimeoutSeconds > 0 && isScreenOn && (millis() - lastActivityTime > (screenTimeoutSeconds * 1000))) {
        
        // Веб-сервер не активен - можно засыпать
        LOG_INFO("Main", "Screen timeout reached. Web server inactive. Entering light sleep.");

        // Отключаем BLE для безопасности, если он активен
        if (currentMode == AppMode::BLE_ADVERTISING || currentMode == AppMode::BLE_PIN_ENTRY || currentMode == AppMode::BLE_CONFIRM_SEND) {
            LOG_INFO("Main", "Disabling BLE due to screen timeout for security");
            bleKeyboardManager.end();
            currentMode = AppMode::PASSWORD;
            bleActionTriggered = false;
        }
        
        // 1. Выключаем дисплей перед сном
        displayManager.turnOff();
        isScreenOn = false;

        // 2. Настраиваем пробуждение только от GPIO 0 (BUTTON_2)
        // Только GPIO 0 является RTC пином и может пробуждать устройство
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // BUTTON_2 пробуждение по низкому уровню
        // Для ESP32-C3 BUTTON_1 (GPIO 35) не может быть использован для пробуждения из глубокого/легкого сна.
        // Мы будем полагаться на пробуждение от BUTTON_2 (GPIO 0) и checkScreenWakeup() для сброса таймера.
        // Это ограничение железа. Для полноценного пробуждения от обеих кнопок нужна другая плата или перепайка.
        // В данном случае, пробуждение будет работать от одной кнопки, а вторая просто включит экран, если устройство уже проснулось.

        LOG_INFO("Main", "Configured wakeup sources. Entering light sleep now.");
        
        // 3. Уходим в легкий сон
        esp_light_sleep_start();
        
        // ... выполнение кода продолжится здесь после пробуждения ...
        LOG_INFO("Main", "Woke up from light sleep.");

        // 立即点亮背光并强制重绘，避免唤醒后黑屏/闪烁/残影
        if (!isScreenOn) {
            wakeDisplaySafely("light_sleep_resume");
        }
        lastActivityTime = millis();
        previousKeyIndex = -1;
        previousPasswordIndex = -1;
        displayManager.hideLoader();
    }

    if (isScreenOn) {
        // Пропускаем обновления если активен лоадер, чтобы предотвратить наслоение
        if (!displayManager.isLoaderActive()) {
            // Обновляем статус батареи по таймеру (общий для всех режимов)
            if (millis() - lastBatteryCheckTime > batteryCheckInterval) {
                lastBatteryCheckTime = millis();
                int currentBatteryPercentage = batteryManager.getPercentage();
                bool isCharging = (batteryManager.getVoltage() > 4.15);
                displayManager.updateBatteryStatus(currentBatteryPercentage, isCharging);
            }
            
            // Мониторинг критического состояния памяти
            static unsigned long lastCriticalMemoryCheck = 0;
            if (millis() - lastCriticalMemoryCheck > 30000) { // Проверяем каждые 30 секунд
                lastCriticalMemoryCheck = millis();
                uint32_t freeHeap = ESP.getFreeHeap();
                
                // Только критические предупреждения для production
                if (freeHeap < 15000) { // Меньше 15KB - критично!
                    LOG_CRITICAL("Memory", "CRITICAL LOW MEMORY! Device may become unstable");
                    
                    // Принудительная очистка кэшей при критической нехватке памяти
                    if (freeHeap < 10000) {
                        ESP.restart(); // Аварийная перезагрузка при < 10KB
                    }
                }
            }
        }

        switch (currentMode) {
            case AppMode::TOTP:
            {
                auto keys = keyManager.getAllKeys();
                if (!keys.empty()) {
                    if (currentKeyIndex != previousKeyIndex) {
                        displayManager.drawLayout(keys[currentKeyIndex].name, batteryManager.getPercentage(), batteryManager.getVoltage() > 4.18, webServerManager.isRunning());
                        previousKeyIndex = currentKeyIndex;
                    }
                    
                    if (!displayManager.isLoaderActive() && millis() - lastTotpUpdateTime > totpUpdateInterval) {
                        lastTotpUpdateTime = millis();
                        
                        // ⚠️ Проверка синхронизации времени
                        if (!totpGenerator.isTimeSynced()) {
                            // Показываем предупреждение вместо TOTP кода (убрали TIME для краткости)
                            displayManager.updateTOTPCode("未同步", 0);
                            
                            // 🌐 Дополнительные предупреждения ТОЛЬКО для WiFi Mode
                            // В AP и Offline режимах - только "TIME 未同步" без подсказок
                            if (WiFi.getMode() == WIFI_STA && WiFi.isConnected()) {
                                // Показываем предупреждения периодически для WiFi режима
                                static unsigned long lastWarningTime = 0;
                                static bool warningsShown = false;
                                
                                // Показываем предупреждения каждые 5 секунд или при смене ключа
                                if (currentKeyIndex != previousKeyIndex || 
                                    millis() - lastWarningTime > 5000 || 
                                    !warningsShown) {
                                    
                                    lastWarningTime = millis();
                                    warningsShown = true;
                                    
                                    // Очистка области предупреждений
                                    TFT_eSPI* tft = displayManager.getTft();
                                    tft->fillRect(0, 115, tft->width(), 60, 
                                                displayManager.getCurrentThemeColors()->background_dark);
                                    
                                    // Отцентрованные предупреждения
                                    tft->setTextDatum(MC_DATUM);
                                    tft->setTextColor(displayManager.getCurrentThemeColors()->text_secondary, 
                                                    displayManager.getCurrentThemeColors()->background_dark);
                                    tft->setTextSize(1);
                                    displayManager.drawUtf8Centered("⚠️ 请连接到网络", tft->width() / 2, 120,
                                                                   displayManager.getCurrentThemeColors()->text_secondary,
                                                                   displayManager.getCurrentThemeColors()->background_dark, true);
                                    displayManager.drawUtf8Centered("以进行时间同步", tft->width() / 2, 135,
                                                                   displayManager.getCurrentThemeColors()->text_secondary,
                                                                   displayManager.getCurrentThemeColors()->background_dark, true);
                                    displayManager.drawUtf8Centered("或切换到密码模式", tft->width() / 2, 150,
                                                                   displayManager.getCurrentThemeColors()->text_secondary,
                                                                   displayManager.getCurrentThemeColors()->background_dark, true);
                                    displayManager.drawUtf8Centered("（长按 BTN1）", tft->width() / 2, 165,
                                                                   displayManager.getCurrentThemeColors()->text_secondary,
                                                                   displayManager.getCurrentThemeColors()->background_dark, true);
                                }
                            }
                        } else {
                            // Время синхронизировано - показываем TOTP код
                            String code = totpGenerator.generateTOTP(keys[currentKeyIndex].secret);
                            int timeLeft = totpGenerator.getTimeRemaining();
                            displayManager.updateTOTPCode(code, timeLeft);
                        }
                    }
                } else {
                    displayManager.drawNoItemsPage("keys");
                }
                break;
            }
            case AppMode::PASSWORD:
            {
                auto passwords = passwordManager.getAllPasswords();
                if (!passwords.empty()) {
                    if (currentPasswordIndex != previousPasswordIndex) {
                        displayManager.drawPasswordLayout(
                            passwords[currentPasswordIndex].name,
                            passwords[currentPasswordIndex].password,
                            batteryManager.getPercentage(),
                            batteryManager.getVoltage() > 4.18,
                            webServerManager.isRunning()
                        );
                        previousPasswordIndex = currentPasswordIndex;
                    }
                } else {
                    displayManager.drawNoItemsPage("passwords");
                }
                break;
            }
            case AppMode::BLE_ADVERTISING:
                {
                    static bool bleInitialized = false;
                    static bool devicePinChecked = false;
                    static unsigned long bleStartTime = 0;
                    
                    if (!bleInitialized) {
                        LOG_INFO("Main", "Entering BLE_ADVERTISING setup");
                        
                        // BLE режим не требует Device PIN - только BLE PIN проверяется позже при передаче
                        LOG_INFO("Main", "BLE mode activated. Device PIN is NOT required for BLE advertising.");
                        devicePinChecked = true;
                        
                        if (devicePinChecked) {
                            if (webServerManager.isRunning()) {
                                webServerManager.stop();
                                LOG_INFO("Main", "Web server stopped");
                            }
                            wifiManager.disconnect();
                            
                            bool bleStarted = bleKeyboardManager.begin();
                            if (bleStarted) {
                                String pinMsg = "PIN：" + String(bleKeyboardManager.getStaticPIN());
                                displayManager.drawBleAdvertisingPage(bleKeyboardManager.getDeviceName(), pinMsg, 0);
                                bleInitialized = true;
                                devicePinChecked = false; // Reset for next time
                            } else {
                                LOG_ERROR("Main", "Failed to start secure BLE");
                                currentMode = AppMode::PASSWORD;
                                bleActionTriggered = false;
                                devicePinChecked = false;
                            }
                            bleStartTime = millis(); // Запоминаем время начала BLE режима
                            LOG_INFO("Main", "BLE Keyboard started. Waiting for connection...");
                        }
                    }

                    if (bleKeyboardManager.isConnected()) {
                        if (bleKeyboardManager.isSecure()) {
                            LOG_INFO("Main", "BLE secure connection established");
                            currentMode = AppMode::BLE_PIN_ENTRY;
                            bleInitialized = false; // Reset for next time
                            bleStartTime = 0;
                        } else {
                            // Защита от спама - логировать только раз в секунду
                            static unsigned long lastAuthLog = 0;
                            static bool pinPageDrawn = false;
                            
                            if (millis() - lastAuthLog > 1000) {
                                LOG_INFO("Main", "BLE connected but not secure - waiting for authentication");
                                lastAuthLog = millis();
                            }
                            
                            if (!pinPageDrawn) {
                                String pinMsg = "请输入 PIN：" + String(bleKeyboardManager.getStaticPIN());
                                displayManager.drawBleAdvertisingPage(bleKeyboardManager.getDeviceName(), pinMsg, 0);
                                pinPageDrawn = true;
                            }
                            
                            // Таймаут аутентификации - 30 секунд
                            if (millis() - bleStartTime > 30000) {
                                LOG_WARNING("Main", "Authentication timeout - disconnecting");
                                bleKeyboardManager.end();
                                currentMode = AppMode::PASSWORD;
                                bleActionTriggered = false;
                                bleInitialized = false;
                                pinPageDrawn = false;
                            }
                        }
                    }

                    // Handle back button press
                    if (digitalRead(BUTTON_1) == LOW) {
                        delay(200); // Debounce
                        LOG_INFO("Main", "Back button pressed - exiting BLE mode");
                        bleKeyboardManager.end();
                        currentMode = AppMode::PASSWORD;
                        bleActionTriggered = false;
                        bleInitialized = false;
                        bleStartTime = 0;
                    }
                }
                break;

            case AppMode::BLE_PIN_ENTRY:
                {
                    LOG_INFO("Main", "BLE secure connection established. Checking BLE PIN requirements...");
                    LOG_DEBUG("Main", "BLE PIN enabled: " + String(pinManager.isPinEnabledForBle()));
                    LOG_DEBUG("Main", "PIN set: " + String(pinManager.isPinSet() ? "yes" : "no"));
                    
                    // Проверяем BLE PIN только если он специально включен для BLE операций
                    bool pinOk = true;
                    if (pinManager.isPinEnabledForBle() && pinManager.isPinSet()) {
                        LOG_INFO("Main", "BLE PIN protection enabled, requesting PIN for transmission...");
                        pinOk = pinManager.requestPin();
                    } else {
                        LOG_INFO("Main", "BLE PIN protection disabled. Proceeding to send confirmation");
                    }
                    
                    if (pinOk) {
                        LOG_INFO("Main", "BLE access granted. Proceeding to send confirmation");
                        currentMode = AppMode::BLE_CONFIRM_SEND;
                    } else {
                        LOG_WARNING("Main", "BLE PIN incorrect or cancelled. Returning to list");
                        bleKeyboardManager.end();
                        currentMode = AppMode::PASSWORD;
                        bleActionTriggered = false;
                    }
                }
                break;

            case AppMode::BLE_CONFIRM_SEND:
                {
                    static bool confirmPageDrawn = false;
                    
                    auto passwords = passwordManager.getAllPasswords();
                    if (passwords.empty() || currentPasswordIndex >= passwords.size()) {
                        // Safety check
                        currentMode = AppMode::PASSWORD;
                        bleKeyboardManager.end();
                        bleActionTriggered = false;
                        confirmPageDrawn = false;
                        break;
                    }
                    
                    // Рисуем страницу только один раз или при принудительной перерисовке
                    if (!confirmPageDrawn || previousPasswordIndex == -1) {
                        String passwordName = passwords[currentPasswordIndex].name;
                        String password = passwords[currentPasswordIndex].password;
                        String deviceName = bleKeyboardManager.getDeviceName();
                        displayManager.drawBleConfirmPage(passwordName, password, deviceName);
                        confirmPageDrawn = true;
                        previousPasswordIndex = currentPasswordIndex;
                    }

                    // Wait for button press
                    if (digitalRead(BUTTON_1) == LOW) { // Back button
                        delay(200);
                        LOG_INFO("Main", "Back button pressed. Cancelling send");
                        currentMode = AppMode::PASSWORD;
                        bleKeyboardManager.end();
                        bleActionTriggered = false;
                        confirmPageDrawn = false;
                    } else if (digitalRead(BUTTON_2) == LOW) { // Send button
                        delay(200);
                        LOG_INFO("Main", "Send button pressed. Sending data");
                        
                        displayManager.drawBleSendingPage();
                        String password = passwords[currentPasswordIndex].password;
                        bleKeyboardManager.sendPassword(password.c_str());
                        delay(500); // Give time for the UI and BLE
                        
                        displayManager.drawBleResultPage(true); // Show success
                        delay(1500);

                        // Возвращаемся к странице подтверждения для повторной отправки
                        LOG_INFO("Main", "Password sent successfully. Returning to confirmation page");
                        previousPasswordIndex = -1; // Force redraw of confirm page
                    }
                }
                break;
        }
    }
    
    // Check for scheduled restart
    if (shouldRestart) {
        LOG_INFO("Main", "Device restart requested. Restarting in 1 second...");
        delay(1000);
        ESP.restart();
    }
}
