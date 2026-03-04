#include "display_manager.h"
#include "config.h"
#include "log_manager.h"
#include <algorithm>

#if __has_include(<U8g2_for_TFT_eSPI.h>)
#include <U8g2_for_TFT_eSPI.h>
#define SECUREGEN_HAS_U8G2 1
#else
#define SECUREGEN_HAS_U8G2 0
#endif

#if SECUREGEN_HAS_U8G2
namespace {
U8g2_for_TFT_eSPI g_u8g2;
bool g_u8g2Initialized = false;

void ensureU8g2Ready(TFT_eSPI& tft) {
    if (!g_u8g2Initialized) {
        g_u8g2.begin(tft);
        g_u8g2Initialized = true;
    }
}
} // namespace
#endif

// Helper for the animation loop
void schedule_next_update(DisplayManager* dm, AnimationManager* am);


bool hasNonAscii(const String& text) {
    for (size_t i = 0; i < text.length(); ++i) {
        if (static_cast<uint8_t>(text[i]) & 0x80) {
            return true;
        }
    }
    return false;
}

void drawUtf8TopLeft(TFT_eSPI& tft, const String& text, int x, int y, uint16_t fg, uint16_t bg) {
#if SECUREGEN_HAS_U8G2
    if (hasNonAscii(text)) {
        ensureU8g2Ready(tft);
        g_u8g2.setFont(u8g2_font_unifont_t_chinese2);
        g_u8g2.setForegroundColor(fg);
        g_u8g2.setBackgroundColor(bg);
        int baselineY = y + g_u8g2.getFontAscent();
        g_u8g2.setCursor(x, baselineY);
        g_u8g2.print(text.c_str());
        return;
    }
#endif
    tft.setTextColor(fg, bg);
    tft.drawString(text, x, y);
}

void animation_callback(float val, bool finished, DisplayManager* dm, AnimationManager* am) {
    dm->updateHeader();
    if (finished) {
        schedule_next_update(dm, am);
    }
}

void schedule_next_update(DisplayManager* dm, AnimationManager* am) {
    am->startAnimation(20, 0.0f, 1.0f, [dm, am](float val, bool finished) {
        animation_callback(val, finished, dm, am);
    });
}


DisplayManager::DisplayManager() : tft(TFT_eSPI()), animationManager(), headerSprite(&tft), totpContainerSprite(&tft), totpSprite(&tft) {
    _currentThemeColors = &DARK_THEME_COLORS;
    _totpState = TotpState::IDLE;
    _lastDrawnTotpString = "";
    _lastScrambleFrameTime = 0;
    _totpContainerNeedsRedraw = true;
}

void DisplayManager::setTheme(Theme theme) {
    LOG_INFO("DisplayManager", "setTheme() called with theme: " + String((theme == Theme::LIGHT) ? "LIGHT" : "DARK"));
    switch (theme) {
        case Theme::DARK:
            _currentThemeColors = &DARK_THEME_COLORS;
            break;
        case Theme::LIGHT:
            _currentThemeColors = &LIGHT_THEME_COLORS;
            break;
    }
    LOG_DEBUG("DisplayManager", "Applying new theme colors to display...");
    tft.fillScreen(_currentThemeColors->background_dark);
    updateHeader(); 
    lastDisplayedCode = ""; 
    lastTimeRemaining = -1;
    _lastDrawnTotpString = ""; 
    _totpState = TotpState::IDLE;
    _totpContainerNeedsRedraw = true; // Force redraw of container with new theme
    LOG_INFO("DisplayManager", "Theme applied successfully");
}

void DisplayManager::update() {
    animationManager.update();

    if (_totpState == TotpState::IDLE) {
        return;
    }

    unsigned long currentTime = millis();
    const unsigned long frameInterval = 30; 

    if (currentTime - _lastScrambleFrameTime < frameInterval) {
        return;
    }
    _lastScrambleFrameTime = currentTime;

    if (_totpContainerNeedsRedraw) {
        drawTotpContainer();
    }

    unsigned long elapsedTime = currentTime - _totpAnimStartTime;
    const unsigned long scrambleDuration = 300;
    const unsigned long revealDuration = 150;
    const unsigned long totalDuration = scrambleDuration + revealDuration;

    if (elapsedTime >= totalDuration) {
        _totpState = TotpState::IDLE;
        drawTotpText(_newCode);
        _currentCode = _newCode;
        return;
    }

    String textToDraw = "";
    const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";

    if (elapsedTime < scrambleDuration) {
        for (int i = 0; i < 6; i++) {
            textToDraw += charset[random(sizeof(charset) - 1)];
        }
    } else {
        int charsToReveal = (elapsedTime - scrambleDuration) / 25;
        textToDraw = _newCode.substring(0, charsToReveal);
        for (int i = charsToReveal; i < 6; i++) {
            textToDraw += charset[random(sizeof(charset) - 1)];
        }
    }
    
    drawTotpText(textToDraw);
}


// Ранняя инициализация для splash screen (минимальная подготовка)
void DisplayManager::initForSplash() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK); // Очищаем экран чёрным для splash
    tft.setTextDatum(MC_DATUM);
    
    // ⚠️ ВАЖНО: Настройка PWM ПОСЛЕ tft.init() т.к. init() сбрасывает пин на digitalWrite!
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 0); // Начинаем с погашенного экрана для fade эффекта
}

// Полная инициализация (для обычного UI)
void DisplayManager::init() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(_currentThemeColors->background_dark); 
    tft.setTextDatum(MC_DATUM);
    
    // ⚠️ ВАЖНО: Настройка PWM ПОСЛЕ tft.init() т.к. init() сбрасывает пин!
    ledcSetup(0, 5000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255); // Полная яркость по умолчанию

    headerSprite.createSprite(tft.width(), 35);
    headerSprite.setTextDatum(MC_DATUM);

    // Создание спрайтов для TOTP
    int padding = 10;
    tft.setTextSize(4);
    int codeAreaWidth = tft.textWidth("888888") + padding * 2;
    int codeAreaHeight = 40 + 10;
    
    // Спрайт контейнера (с тенью)
    totpContainerSprite.createSprite(codeAreaWidth + 2, codeAreaHeight + 2);
    totpContainerSprite.setTextDatum(MC_DATUM);

    // Спрайт для текста (помещается внутри рамки)
    totpSprite.createSprite(codeAreaWidth - 2, codeAreaHeight - 2);
    totpSprite.setTextDatum(MC_DATUM);


    _totpState = TotpState::IDLE;
    _lastDrawnTotpString = "";
    _totpContainerNeedsRedraw = true;

    schedule_next_update(this, &animationManager);
}

void DisplayManager::drawLayout(const String& serviceName, int batteryPercentage, bool isCharging, bool isWebServerOn) {
    // Если до этого была страница "нет ключей", очищаем экран полностью
    if (_isNoItemsPageActive) {
        tft.fillScreen(_currentThemeColors->background_dark);
        _isNoItemsPageActive = false;
    } else if (_isKeySwitched) {
        // Иначе, если ключ был переключен, очищаем только область под заголовком
        tft.fillRect(0, headerSprite.height(), tft.width(), tft.height() - headerSprite.height(), _currentThemeColors->background_dark);
    }
    
    _currentServiceName = serviceName;
    _currentBatteryPercentage = batteryPercentage;
    _isCharging = isCharging;
    _isWebServerOn = isWebServerOn;
    _headerState = HeaderState::INTRO;
    _introAnimStartTime = millis();

    lastDisplayedCode = ""; 
    lastTimeRemaining = -1;
    _lastDrawnTotpString = "";
    _totpState = TotpState::IDLE;
    _totpContainerNeedsRedraw = true;
    // _isKeySwitched флаг устанавливается в main.cpp
}

void DisplayManager::updateBatteryStatus(int percentage, bool isCharging) {
    _currentBatteryPercentage = percentage;
    _isCharging = isCharging;

    if (_headerState != HeaderState::INTRO) {
        if (_isCharging) {
            if (_headerState != HeaderState::CHARGING) {
                _headerState = HeaderState::CHARGING;
                _chargingAnimStartTime = millis();
            }
        } else {
            _headerState = HeaderState::STATIC;
        }
    }
    
    // 🔋 Обновляем индикатор батареи на странице "No keys found" без полной перерисовки
    if (_isNoItemsPageActive) {
        int batteryX = tft.width() - 28;
        int batteryY = 5;
        int batteryWidth = 22;
        int batteryHeight = 12;
        int batteryCornerRadius = 3;
        int shadowOffset = 1;
        
        // Очищаем область батареи
        tft.fillRect(batteryX - 2, batteryY - 2, batteryWidth + 6, batteryHeight + 4, _currentThemeColors->background_dark);
        
        // Тень батареи
        tft.drawRoundRect(batteryX + shadowOffset, batteryY + shadowOffset, batteryWidth, batteryHeight, batteryCornerRadius, _currentThemeColors->shadow_color);
        tft.fillRect(batteryX + batteryWidth + shadowOffset, batteryY + 3 + shadowOffset, 2, 5, _currentThemeColors->shadow_color);
        
        // Обводка батареи
        tft.drawRoundRect(batteryX, batteryY, batteryWidth, batteryHeight, batteryCornerRadius, _currentThemeColors->text_secondary);
        tft.fillRect(batteryX + batteryWidth, batteryY + 3, 2, 5, _currentThemeColors->text_secondary);
        
        // Заполнение батареи
        uint16_t barColor = (percentage >= 20) ? _currentThemeColors->accent_primary : _currentThemeColors->error_color;
        int barWidth = map(percentage, 0, 100, 0, batteryWidth - 4);
        if (barWidth > 0) {
            tft.fillRect(batteryX + 2, batteryY + 2, barWidth, batteryHeight - 4, barColor);
        }
    }
}

void DisplayManager::updateHeader() {
    // Не отрисовываем заголовок если активен лоадер, чтобы предотвратить наслоение
    if (_loaderActive) {
        return;
    }
    
    // 🚫 Не отрисовываем заголовок на странице "No keys found" - шторка загораживает обводку!
    if (_isNoItemsPageActive) {
        return;
    }
    
    headerSprite.fillSprite(_currentThemeColors->background_dark);

    float titleY = 20;
    
    if (_headerState == HeaderState::INTRO) {
        unsigned long elapsedTime = millis() - _introAnimStartTime;
        if (elapsedTime >= 350) {
            titleY = 20;
            _headerState = _isCharging ? HeaderState::CHARGING : HeaderState::STATIC;
            if (_isCharging) _chargingAnimStartTime = millis();
        } else {
            float progress = (float)elapsedTime / 350.0f;
            titleY = -20.0f + (40.0f * progress);
        }
    }

    headerSprite.setTextColor(_currentThemeColors->text_primary, _currentThemeColors->background_dark);
    headerSprite.setTextSize(2);
    const bool titleHasUtf8 = hasNonAscii(_currentServiceName);
    if (!titleHasUtf8) {
        headerSprite.drawString(_currentServiceName, headerSprite.width() / 2, (int)titleY);
    }

    // Draw WiFi Icon
    if (_isWebServerOn) {
        int wifiX = headerSprite.width() - 55;
        int wifiY = 10;
        headerSprite.drawLine(wifiX, wifiY + 8, wifiX + 8, wifiY, _currentThemeColors->text_secondary);
        headerSprite.drawLine(wifiX + 1, wifiY + 8, wifiX + 8, wifiY + 1, _currentThemeColors->text_secondary);
        headerSprite.drawCircle(wifiX + 4, wifiY + 10, 2, _currentThemeColors->text_secondary);
    }

    if (_headerState == HeaderState::CHARGING) {
        unsigned long chargeElapsedTime = millis() - _chargingAnimStartTime;
        int chargeProgress = (chargeElapsedTime % 1500) * 100 / 1500;
        drawBatteryOnSprite(_currentBatteryPercentage, true, chargeProgress);
    } else {
        drawBatteryOnSprite(_currentBatteryPercentage, false);
    }

    headerSprite.pushSprite(0, 0);

    // UTF-8 标题直接绘制到主屏，避免 sprite ASCII 字体导致中文缺字/错位
    if (titleHasUtf8) {
        drawUtf8Centered(_currentServiceName, tft.width() / 2, (int)titleY, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
    }
}

void DisplayManager::drawBatteryOnSprite(int percentage, bool isCharging, int chargingValue) {
    int x = headerSprite.width() - 28;
    int y = 5;
    int width = 22;
    int height = 11;
    int shadowOffset = 1;
    int cornerRadius = 3;

    headerSprite.drawRoundRect(x + shadowOffset, y + shadowOffset, width, height, cornerRadius, _currentThemeColors->shadow_color);
    headerSprite.fillRect(x + width + shadowOffset, y + 3 + shadowOffset, 2, 5, _currentThemeColors->shadow_color);

    headerSprite.drawRoundRect(x, y, width, height, cornerRadius, _currentThemeColors->text_secondary);
    headerSprite.fillRect(x + width, y + 3, 2, 5, _currentThemeColors->text_secondary);

    uint16_t barColor;
    int barWidth;

    if (percentage > 50) barColor = _currentThemeColors->accent_primary;
    else if (percentage > 20) barColor = _currentThemeColors->accent_secondary;
    else barColor = _currentThemeColors->error_color;
    barWidth = map(percentage, 0, 100, 0, width - 4);

    if (barWidth > 0) {
        headerSprite.fillRect(x + 2, y + 2, barWidth, height - 4, barColor);
    }
}

void DisplayManager::drawTotpContainer() {
    int cornerRadius = 8;
    int containerW = totpContainerSprite.width() - 2;
    int containerH = totpContainerSprite.height() - 2;

    // 1. Очищаем спрайт контейнера
    totpContainerSprite.fillSprite(_currentThemeColors->background_dark);

    // 2. Рисуем тень со смещением
    totpContainerSprite.fillRoundRect(2, 2, containerW, containerH, cornerRadius, _currentThemeColors->shadow_color);
    
    // 3. Рисуем основной фон контейнера
    totpContainerSprite.fillRoundRect(0, 0, containerW, containerH, cornerRadius, _currentThemeColors->background_light);
    
    // 4. Рисуем обводку поверх фона
    totpContainerSprite.drawRoundRect(0, 0, containerW, containerH, cornerRadius, _currentThemeColors->text_secondary);
    
    _totpContainerNeedsRedraw = false;
    _lastDrawnTotpString = ""; 
}

void DisplayManager::drawTotpText(const String& textToDraw) {
    if (textToDraw == _lastDrawnTotpString && !_totpContainerNeedsRedraw) {
        return;
    }

    // 1. Рисуем анимированный текст в свой спрайт
    totpSprite.fillSprite(_currentThemeColors->background_light);
    totpSprite.setTextColor(_currentThemeColors->text_primary, _currentThemeColors->background_light);

    const bool useUtf8Path = hasNonAscii(textToDraw);
    // Уменьшаем размер шрифта для длинного текста (например "NOT SYNCED")
    int textSize = (textToDraw.length() > 6) ? 2 : 4;
    if (!useUtf8Path) {
        totpSprite.setTextSize(textSize);
        totpSprite.drawString(textToDraw, totpSprite.width() / 2, totpSprite.height() / 2);
    }

    // 2. Накладываем спрайт с текстом внутрь рамки контейнера со смещением в 1px
    totpSprite.pushToSprite(&totpContainerSprite, 1, 1);

    // 3. Выводим финальный спрайт контейнера на экран
    int centerX = tft.width() / 2;
    int codeY = tft.height() / 2;
    int containerX = centerX - totpContainerSprite.width() / 2;
    int containerY = codeY - totpContainerSprite.height() / 2;
    totpContainerSprite.pushSprite(containerX, containerY);

    // 4. UTF-8 文本直接绘制到主屏，避免 sprite+ASCII 字体导致乱码
    if (useUtf8Path) {
        drawUtf8Centered(textToDraw, centerX, codeY, _currentThemeColors->text_primary, _currentThemeColors->background_light, true);
    }

    _lastDrawnTotpString = textToDraw;
}


void DisplayManager::updateTOTPCode(const String& code, int timeRemaining) {
    if (_totpContainerNeedsRedraw) {
        drawTotpContainer();
    }

    // Если ключ был только что переключен, просто обновляем код без анимации
    if (_isKeySwitched) {
        _currentCode = code;
        _newCode = code;
        _totpState = TotpState::IDLE;
        _isKeySwitched = false; // Сбрасываем флаг
    }

    // 中文/UTF-8 状态文本（例如“未同步”）不走打乱动画，避免多字节截断导致乱码
    const bool shouldBypassAnimation = hasNonAscii(code) || hasNonAscii(_currentCode);

    // Логика запуска анимации, если код изменился и это не переключение ключа
    if (code != _currentCode && _totpState == TotpState::IDLE) {
        if (_currentCode.length() > 0 && !shouldBypassAnimation) {
            _newCode = code;
            _totpState = TotpState::SCRAMBLING;
            _totpAnimStartTime = millis();
            _lastScrambleFrameTime = millis();
        } else {
            _currentCode = code; // Первый код/UTF-8 状态 устанавливается без анимации
        }
    }
    
    // Если не анимируемся, просто рисуем текущий код
    if (_totpState == TotpState::IDLE) {
        drawTotpText(_currentCode);
    }

    // Обновление прогресс-бара времени
    if (timeRemaining != lastTimeRemaining) {
        int barY = tft.height() - 30;
        int barHeight = 10;
        int barWidth = (tft.width() - 64) * 0.8;
        int barX = (tft.width() - barWidth) / 2;
        int shadowOffset = 2;
        int barCornerRadius = 5;

        // Очищаем область прогресс-бара
        tft.fillRect(barX - shadowOffset, barY - shadowOffset, barWidth + 40 + shadowOffset, barHeight + shadowOffset * 2, _currentThemeColors->background_dark);
        
        // Рисуем рамку и фон
        tft.fillRoundRect(barX + shadowOffset, barY + shadowOffset, barWidth, barHeight, barCornerRadius, _currentThemeColors->shadow_color);
        tft.drawRoundRect(barX, barY, barWidth, barHeight, barCornerRadius, _currentThemeColors->text_secondary);
        tft.fillRoundRect(barX, barY, barWidth, barHeight, barCornerRadius, _currentThemeColors->background_light);

        // Рисуем заполнение
        int fillWidth = map(timeRemaining, CONFIG_TOTP_STEP_SIZE, 0, barWidth, 0);
        tft.fillRoundRect(barX, barY, fillWidth, barHeight, barCornerRadius, _currentThemeColors->accent_primary);

        // Рисуем текст времени
        tft.setTextColor(_currentThemeColors->text_secondary, _currentThemeColors->background_dark);
        tft.setTextSize(2);
        tft.drawString(String(timeRemaining) + "s", barX + barWidth + 20, barY + barHeight / 2);
        
        lastTimeRemaining = timeRemaining;
    }
}

void DisplayManager::showMessage(const String& text, int x, int y, bool isError, int size) {
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(x, y);
    tft.setTextSize(size);
    tft.setTextColor(isError ? _currentThemeColors->error_color : _currentThemeColors->text_primary, _currentThemeColors->background_dark);
    drawUtf8TopLeft(tft, text, x, y, isError ? _currentThemeColors->error_color : _currentThemeColors->text_primary, _currentThemeColors->background_dark);
    tft.setTextDatum(MC_DATUM);
}

void DisplayManager::showMessage(const String& text, int x, int y, bool isError, int size, bool inverted) {
    tft.setTextDatum(TL_DATUM);
    tft.setCursor(x, y);
    tft.setTextSize(size);
    if (inverted) {
        drawUtf8TopLeft(tft, text, x, y, _currentThemeColors->background_dark, _currentThemeColors->text_primary);
    } else {
        drawUtf8TopLeft(tft, text, x, y, isError ? _currentThemeColors->error_color : _currentThemeColors->text_primary, _currentThemeColors->background_dark);
    }
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary, _currentThemeColors->background_dark);
}

void DisplayManager::turnOff() { ledcWrite(0, 0); }
void DisplayManager::turnOn() { ledcWrite(0, 255); }
void DisplayManager::setBrightness(uint8_t brightness) { ledcWrite(0, brightness); }

// 🔄 Обновление текста без полной перерисовки экрана
void DisplayManager::updateMessage(const String& text, int x, int y, int size) {
    tft.setTextSize(size);
    tft.setTextDatum(TL_DATUM);

    // 清空从 x 到屏幕右侧的一整行区域，避免中英文切换时残影/截断
    int textHeight = tft.fontHeight() * size;
#if SECUREGEN_HAS_U8G2
    if (hasNonAscii(text)) {
        ensureU8g2Ready(tft);
        g_u8g2.setFont(u8g2_font_unifont_t_chinese2);
        textHeight = max(textHeight, (int)(g_u8g2.getFontAscent() - g_u8g2.getFontDescent()) + 8);
    }
#endif
    tft.fillRect(x, y, tft.width() - x, textHeight + 6, _currentThemeColors->background_dark);

    // Рисуем новый текст
    drawUtf8TopLeft(tft, text, x, y, _currentThemeColors->text_primary, _currentThemeColors->background_dark);

    tft.setTextDatum(MC_DATUM);
}

// 🧽 Очистка конкретной области
void DisplayManager::clearMessageArea(int x, int y, int width, int height) {
    tft.fillRect(x, y, width, height, _currentThemeColors->background_dark);
}

TFT_eSPI* DisplayManager::getTft() { return &tft; }
void DisplayManager::fillRect(int32_t x, int32_t t, int32_t w, int32_t h, uint32_t color) { tft.fillRect(x, t, w, h, color); }

void DisplayManager::drawUtf8Centered(const String& text, int x, int y, uint16_t fg, uint16_t bg, bool compact) {
#if SECUREGEN_HAS_U8G2
    ensureU8g2Ready(tft);
    g_u8g2.setFont(u8g2_font_unifont_t_chinese2);
    g_u8g2.setForegroundColor(fg);
    g_u8g2.setBackgroundColor(bg);

    int textWidth = g_u8g2.getUTF8Width(text.c_str());
    int baselineY = y + ((g_u8g2.getFontAscent() - g_u8g2.getFontDescent()) / 2);
    int drawX = x - textWidth / 2;

    if (compact) {
        baselineY -= 2;
    }

    g_u8g2.setCursor(drawX, baselineY);
    g_u8g2.print(text.c_str());
#else
    (void)compact;
    tft.setTextColor(fg, bg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(text, x, y);
#endif
}

bool DisplayManager::promptWebServerSelection() {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextColor(_currentThemeColors->text_primary);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    drawUtf8Centered("启动 Web 服务?", tft.width() / 2, 30, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);

    bool selection = true; // true for "Yes", false for "No"
    
    auto drawButtons = [&](bool currentSelection) {
        int btnWidth = 80;
        int btnHeight = 40;
        int btnY = tft.height() / 2 + 10;
        int yesX = tft.width() / 2 - btnWidth - 10;
        int noX = tft.width() / 2 + 10;

        // Очищаем область кнопок перед перерисовкой
        tft.fillRect(yesX - 2, btnY - 2, btnWidth + 4, btnHeight + 4, _currentThemeColors->background_dark);
        tft.fillRect(noX - 2, btnY - 2, btnWidth + 4, btnHeight + 4, _currentThemeColors->background_dark);

        // Draw "Yes" button
        if (currentSelection) {
            tft.fillRoundRect(yesX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->accent_primary);
            tft.setTextColor(_currentThemeColors->background_dark);
        } else {
            tft.fillRoundRect(yesX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->background_dark);
            tft.drawRoundRect(yesX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->text_secondary);
            tft.setTextColor(_currentThemeColors->text_primary);
        }
        drawUtf8Centered("是", yesX + btnWidth / 2, btnY + btnHeight / 2,
                         currentSelection ? _currentThemeColors->background_dark : _currentThemeColors->text_primary,
                         currentSelection ? _currentThemeColors->accent_primary : _currentThemeColors->background_dark, true);

        // Draw "No" button
        if (!currentSelection) {
            tft.fillRoundRect(noX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->accent_primary);
            tft.setTextColor(_currentThemeColors->background_dark);
        } else {
            tft.fillRoundRect(noX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->background_dark);
            tft.drawRoundRect(noX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->text_secondary);
            tft.setTextColor(_currentThemeColors->text_primary);
        }
        drawUtf8Centered("否", noX + btnWidth / 2, btnY + btnHeight / 2,
                         !currentSelection ? _currentThemeColors->background_dark : _currentThemeColors->text_primary,
                         !currentSelection ? _currentThemeColors->accent_primary : _currentThemeColors->background_dark, true);
        
        // Reset text color to default for other text elements
        tft.setTextColor(_currentThemeColors->text_primary);
    };

    drawButtons(selection);

    unsigned long startTime = millis();
    const unsigned long timeout = 8000; // 8 秒，便于用户看清并操作

    while (millis() - startTime < timeout) {
        // Button 1 (top, GPIO 35) to toggle
        if (digitalRead(BUTTON_1) == LOW) {
            selection = !selection;
            drawButtons(selection);
            delay(300); // Debounce
            startTime = millis(); // Reset timeout on activity
        }

        // Button 2 (bottom, GPIO 0) to confirm
        if (digitalRead(BUTTON_2) == LOW) {
            delay(300); // Debounce
            
            // 🧹 Очистка экрана перед возвратом
            tft.fillScreen(_currentThemeColors->background_dark);
            delay(50); // Даем дисплею время на обновление
            
            return selection;
        }
        delay(50);
    }

    // 🧹 КРИТИЧНО: Очистка экрана после таймаута
    // Без этого кнопки промптинга остаются на экране!
    tft.fillScreen(_currentThemeColors->background_dark);
    
    return false; // Default to "No" after timeout
}

// 🌌 Промптинг выбора режима запуска (AP/Offline/WiFi)
StartupMode DisplayManager::promptModeSelection() {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextColor(_currentThemeColors->text_primary);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    drawUtf8Centered("选择模式", tft.width() / 2, 20, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
    
    // Подзаголовок с подсказкой
    tft.setTextSize(1);
    tft.setTextColor(_currentThemeColors->text_secondary);
    drawUtf8Centered("自动: WiFi 模式(默认)", tft.width() / 2, 45, _currentThemeColors->text_secondary, _currentThemeColors->background_dark, true);

    bool selection = true; // true для AP, false для Offline
    
    auto drawButtons = [&](bool currentSelection) {
        int btnWidth = 80;
        int btnHeight = 40;
        int btnY = tft.height() / 2 + 10;
        int apX = tft.width() / 2 - btnWidth - 10;
        int offlineX = tft.width() / 2 + 10;

        // Очистка области кнопок
        tft.fillRect(apX - 2, btnY - 2, btnWidth + 4, btnHeight + 4, _currentThemeColors->background_dark);
        tft.fillRect(offlineX - 2, btnY - 2, btnWidth + 4, btnHeight + 4, _currentThemeColors->background_dark);

        // 🔘 Кнопка "AP"
        if (currentSelection) {
            tft.fillRoundRect(apX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->accent_primary);
            tft.setTextColor(_currentThemeColors->background_dark);
        } else {
            tft.fillRoundRect(apX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->background_dark);
            tft.drawRoundRect(apX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->text_secondary);
            tft.setTextColor(_currentThemeColors->text_secondary);
        }
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(2);
        tft.drawString("AP", apX + btnWidth/2, btnY + btnHeight/2);

        // 🔘 Кнопка "Offline"
        if (!currentSelection) {
            tft.fillRoundRect(offlineX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->accent_primary);
            tft.setTextColor(_currentThemeColors->background_dark);
        } else {
            tft.fillRoundRect(offlineX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->background_dark);
            tft.drawRoundRect(offlineX, btnY, btnWidth, btnHeight, 8, _currentThemeColors->text_secondary);
            tft.setTextColor(_currentThemeColors->text_secondary);
        }
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        drawUtf8Centered("离线", offlineX + btnWidth/2, btnY + btnHeight/2,
                         !currentSelection ? _currentThemeColors->background_dark : _currentThemeColors->text_secondary,
                         !currentSelection ? _currentThemeColors->accent_primary : _currentThemeColors->background_dark, true);
        
        // Сброс цвета текста
        tft.setTextColor(_currentThemeColors->text_primary);
    };

    drawButtons(selection);

    unsigned long startTime = millis();
    const unsigned long timeout = 8000; // 8 秒，便于用户看清并操作

    while (millis() - startTime < timeout) {
        // Button 1 (GPIO 35) - переключение между AP/Offline
        if (digitalRead(BUTTON_1) == LOW) {
            selection = !selection;
            drawButtons(selection);
            delay(300); // Debounce
            startTime = millis(); // Сброс таймаута при активности
        }

        // Button 2 (GPIO 0) - подтверждение выбора
        if (digitalRead(BUTTON_2) == LOW) {
            delay(300); // Debounce
            
            // 🧹 Очистка экрана перед переходом к выбранному режиму
            tft.fillScreen(_currentThemeColors->background_dark);
            delay(50); // Даем дисплею время на обновление
            
            return selection ? StartupMode::AP_MODE : StartupMode::OFFLINE_MODE;
        }
        delay(50);
    }

    // 🧹 КРИТИЧНО: Очистка экрана после таймаута перед WiFi Mode
    // Без этого текст "Connecting WiFi..." рисуется ПОВЕРХ промптинга!
    tft.fillScreen(_currentThemeColors->background_dark);
    
    return StartupMode::WIFI_MODE; // По умолчанию WiFi Mode после таймаута
}

void DisplayManager::drawPasswordLayout(const String& name, const String& password, int batteryPercentage, bool isCharging, bool isWebServerOn) {
    if (_isNoItemsPageActive) {
        _isNoItemsPageActive = false;
        tft.fillScreen(_currentThemeColors->background_dark); 
    } else {
        // Очищаем только область контента под заголовком, чтобы избежать мерцания
        tft.fillRect(0, headerSprite.height(), tft.width(), tft.height() - headerSprite.height(), _currentThemeColors->background_dark);
    }
    
    _currentServiceName = name; // Используем ту же переменную для имени
    _currentBatteryPercentage = batteryPercentage;
    _isCharging = isCharging;
    _isWebServerOn = isWebServerOn;
    _headerState = HeaderState::INTRO;
    _introAnimStartTime = millis();

    // Отрисовка частично замаскированного пароля
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary, _currentThemeColors->background_dark);
    tft.setTextSize(2); // Меньший размер шрифта
    
    // Создаем частично замаскированную версию пароля
    String maskedPassword = "";
    if (password.length() == 0) {
        maskedPassword = "[空]";
    } else if (password.length() == 1) {
        maskedPassword = password.substring(0, 1) + "*";
    } else if (password.length() == 2) {
        maskedPassword = password;
    } else {
        // Показываем первые 2 символа + звездочки для остальных
        maskedPassword = password.substring(0, 2);
        int remainingChars = password.length() - 2;
        for (int i = 0; i < remainingChars && i < 10; i++) { // Ограничиваем количество звездочек
            maskedPassword += "*";
        }
        if (remainingChars > 10) {
            maskedPassword += "...";
        }
    }
    
    tft.drawString(maskedPassword, tft.width() / 2, tft.height() / 2);

    lastDisplayedCode = ""; 
    lastTimeRemaining = -1;
    _lastDrawnTotpString = "";
    _totpState = TotpState::IDLE;
    _totpContainerNeedsRedraw = true;
}

void DisplayManager::drawBleInitLoader(int progress) {
    drawGenericLoader(progress, "正在启动 BLE...");
}

void DisplayManager::drawGenericLoader(int progress, const String& text) {
    // Оптимизированная перерисовка - перерисовываем только при изменениях
    bool needsFullRedraw = !_loaderActive || _lastLoaderText != text;
    bool needsProgressUpdate = _lastLoaderProgress != progress;
    
    if (needsFullRedraw) {
        // Полная перерисовка с фоном согласно теме для предотвращения миганий
        tft.fillScreen(_currentThemeColors->background_dark);
        
        // Рисуем основные элементы с цветами темы
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(_currentThemeColors->text_primary);
        tft.setTextSize(2);
        
        // Заголовок
        int textY = tft.height() / 2 - 30;
        drawUtf8Centered(text, tft.width() / 2, textY, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
        
        // Рамка прогресс-бара (один раз)
        int barWidth = 100;
        int barHeight = 12;
        int barX = (tft.width() - barWidth) / 2;
        int barY = tft.height() / 2 + 5;
        
        // Фон под прогресс-баром согласно теме для предотвращения артефактов
        tft.fillRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4, _currentThemeColors->background_dark);
        
        // Рамка прогресс-бара с цветом темы
        tft.drawRect(barX, barY, barWidth, barHeight, _currentThemeColors->text_secondary);
        
        _lastLoaderText = text;
        _loaderActive = true;
    }
    
    if (needsProgressUpdate) {
        // Обновляем только заливку прогресс-бара
        int barWidth = 100;
        int barHeight = 12;
        int barX = (tft.width() - barWidth) / 2;
        int barY = tft.height() / 2 + 5;
        
        // Очищаем внутренность бара фоном темы
        tft.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, _currentThemeColors->background_dark);
        
        // Рисуем новую заливку с акцентным цветом темы
        int fillWidth = (barWidth - 2) * progress / 100;
        if (fillWidth > 0) {
            tft.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, _currentThemeColors->accent_primary);
        }
        
        _lastLoaderProgress = progress;
    }
}

void DisplayManager::hideLoader() {
    if (!_loaderActive) return; // Не делаем ничего, если лоадер не был активен
    _loaderActive = false;
    _lastLoaderText = "";
    _lastLoaderProgress = -1;
    _isNoItemsPageActive = false; // Сбрасываем флаг, чтобы вызвать полную перерисовку
    tft.fillScreen(_currentThemeColors->background_dark); // Очищаем экран
}

void DisplayManager::drawBleAdvertisingPage(const String& deviceName, const String& status, int timeLeft) {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary);
    tft.setTextSize(2);
    drawUtf8Centered("BLE 广播中", tft.width() / 2, 20, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
    
    tft.setTextSize(1);
    drawUtf8Centered("设备名称:", tft.width() / 2, 50, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
    tft.setTextSize(2);
    drawUtf8Centered(deviceName, tft.width() / 2, 70, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);

    tft.setTextSize(1);
    drawUtf8Centered(status, tft.width() / 2, 100, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);

    // Button labels
    tft.setTextSize(1);
    drawUtf8Centered("返回", 30, tft.height() - 20, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
}

void DisplayManager::drawBleConfirmPage(const String& passwordName, const String& password, const String& deviceName) {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary);
    
    // Имя пароля сверху
    tft.setTextSize(1);
    tft.setTextColor(_currentThemeColors->text_secondary);
    String displayName = passwordName;
    if (!hasNonAscii(displayName) && displayName.length() > 20) {
        displayName = displayName.substring(0, 20) + "...";
    }
    drawUtf8Centered(displayName, tft.width() / 2, 15, _currentThemeColors->text_secondary, _currentThemeColors->background_dark, true);
    
    // Замаскированный пароль в центре
    tft.setTextSize(2);
    tft.setTextColor(_currentThemeColors->text_primary);
    String maskedPassword = "";
    for (int i = 0; i < password.length() && i < 12; i++) {
        maskedPassword += "*";
    }
    if (password.length() > 12) {
        maskedPassword += "...";
    }
    tft.drawString(maskedPassword, tft.width() / 2, tft.height() / 2 - 5);
    
    // Информация об устройстве
    tft.setTextSize(1);
    tft.setTextColor(_currentThemeColors->accent_primary);
    drawUtf8Centered("BLE 已连接", tft.width() / 2, tft.height() / 2 + 20, _currentThemeColors->accent_primary, _currentThemeColors->background_dark, true);

    // Кнопки внизу
    tft.setTextSize(1);
    tft.setTextColor(_currentThemeColors->text_secondary);
    drawUtf8TopLeft(tft, "返回", 8, tft.height() - 20, _currentThemeColors->text_secondary, _currentThemeColors->background_dark);

    int sendTextWidth = tft.textWidth("发送");
#if SECUREGEN_HAS_U8G2
    ensureU8g2Ready(tft);
    g_u8g2.setFont(u8g2_font_unifont_t_chinese2);
    sendTextWidth = g_u8g2.getUTF8Width("发送");
#endif
    int sendX = tft.width() - sendTextWidth - 8;
    drawUtf8TopLeft(tft, "发送", sendX, tft.height() - 20, _currentThemeColors->text_secondary, _currentThemeColors->background_dark);

    // Восстанавливаем центральное выравнивание
    tft.setTextDatum(MC_DATUM);
}

void DisplayManager::drawBleSendingPage() {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary);
    tft.setTextSize(2);
    drawUtf8Centered("发送中...", tft.width() / 2, tft.height() / 2, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
}

void DisplayManager::drawBleResultPage(bool success) {
    tft.fillScreen(_currentThemeColors->background_dark);
    tft.setTextDatum(MC_DATUM);
    if (success) {
        tft.setTextColor(_currentThemeColors->accent_primary);
        tft.setTextSize(2);
        drawUtf8Centered("发送成功!", tft.width() / 2, tft.height() / 2, _currentThemeColors->accent_primary, _currentThemeColors->background_dark, true);
    } else {
        tft.setTextColor(_currentThemeColors->error_color);
        tft.setTextSize(2);
        drawUtf8Centered("发送失败", tft.width() / 2, tft.height() / 2, _currentThemeColors->error_color, _currentThemeColors->background_dark, true);
    }
}

void DisplayManager::drawNoItemsPage(const String& text) {
    if (_isNoItemsPageActive) {
        // Если страница уже отображается, ничего не делаем, чтобы избежать мерцания.
        // Обновление батареи будет происходить отдельно в main loop.
        return;
    }

    _isNoItemsPageActive = true;
    tft.fillScreen(_currentThemeColors->background_dark);
    
    // 🔋 Рисуем индикатор батареи в правом верхнем углу
    int batteryX = tft.width() - 28;
    int batteryY = 5;
    int batteryWidth = 22;
    int batteryHeight = 12;
    int batteryCornerRadius = 3;
    int shadowOffset = 1;
    
    // Тень батареи
    tft.drawRoundRect(batteryX + shadowOffset, batteryY + shadowOffset, batteryWidth, batteryHeight, batteryCornerRadius, _currentThemeColors->shadow_color);
    tft.fillRect(batteryX + batteryWidth + shadowOffset, batteryY + 3 + shadowOffset, 2, 5, _currentThemeColors->shadow_color);
    
    // Обводка батареи
    tft.drawRoundRect(batteryX, batteryY, batteryWidth, batteryHeight, batteryCornerRadius, _currentThemeColors->text_secondary);
    tft.fillRect(batteryX + batteryWidth, batteryY + 3, 2, 5, _currentThemeColors->text_secondary);
    
    // Заполнение батареи
    uint16_t barColor;
    int barWidth;
    if (_currentBatteryPercentage >= 20) {
        barColor = _currentThemeColors->accent_primary;
    } else {
        barColor = _currentThemeColors->error_color;
    }
    barWidth = map(_currentBatteryPercentage, 0, 100, 0, batteryWidth - 4);
    if (barWidth > 0) {
        tft.fillRect(batteryX + 2, batteryY + 2, barWidth, batteryHeight - 4, barColor);
    }
    
    // 📡 Рисуем WiFi иконку если веб-сервер включен
    if (_isWebServerOn) {
        int wifiX = tft.width() - 55;
        int wifiY = 10;
        tft.drawLine(wifiX, wifiY + 8, wifiX + 8, wifiY, _currentThemeColors->text_secondary);
        tft.drawLine(wifiX + 1, wifiY + 8, wifiX + 8, wifiY + 1, _currentThemeColors->text_secondary);
        tft.fillCircle(wifiX + 4, wifiY + 10, 2, _currentThemeColors->text_secondary);
    }
    
    // 📏 Размеры округленной рамки (в области TOTP кода)
    int boxWidth = 180;
    int boxHeight = 70;
    int boxX = (tft.width() - boxWidth) / 2;
    int boxY = tft.height() / 2 - boxHeight / 2;
    int cornerRadius = 12;
    
    // 🔲 Рисуем округленную рамку
    tft.drawRoundRect(boxX, boxY, boxWidth, boxHeight, cornerRadius, _currentThemeColors->text_secondary);
    
    // 📝 Текст внутри рамки (меньше размер, отцентрован)
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(_currentThemeColors->text_primary);
    tft.setTextSize(1); // Уменьшенный размер
    
    String line1 = "未找到 " + text;
    drawUtf8Centered(line1, tft.width() / 2, tft.height() / 2 - 12, _currentThemeColors->text_primary, _currentThemeColors->background_dark, true);
    
    tft.setTextColor(_currentThemeColors->text_secondary);
    drawUtf8Centered("请在 Web 界面添加", tft.width() / 2, tft.height() / 2 + 8, _currentThemeColors->text_secondary, _currentThemeColors->background_dark, true);
}
