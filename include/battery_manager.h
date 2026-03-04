#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include "esp_adc_cal.h" // Required for ADC calibration
#include "driver/adc.h"  // Required for ADC driver functions

class BatteryManager {
public:
    BatteryManager(int adcPin, int powerPin);
    void begin();
    
    // ⚡ OPTIMIZED: Возвращаем милливольты (integer) вместо float вольтов
    uint32_t getVoltageMv();  // 5x быстрее чем getVoltage()
    float getVoltage();        // Deprecated: оставляем для совместимости
    int getPercentage();

private:
    int _adcPin;
    int _powerPin;
    // ⚡ OPTIMIZED: Калибровка в милливольтах (integer math)
    // 使用更接近锂电池真实工作区间的映射，避免“看起来还有一半电”但实际已偏低
    const uint32_t _maxVoltageMv = 4200;  // 4.2V = 100%
    const uint32_t _minVoltageMv = 3000;  // 3.0V = 0%
    
    // Deprecated float версии (для совместимости)
    const float _maxVoltage = 4.2;
    const float _minVoltage = 3.0;

    esp_adc_cal_characteristics_t _adc_chars; // ADC calibration characteristics

    // ⚡ OPTIMIZED: Integer map function (5-10x быстрее float)
    int32_t imap(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
    
    // Deprecated: float версия (оставляем для совместимости)
    float fmap(float x, float in_min, float in_max, float out_min, float out_max);
};

#endif // BATTERY_MANAGER_H
