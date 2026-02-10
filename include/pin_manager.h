#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#include "display_manager.h"

#define DEFAULT_PIN_LENGTH 6
#define MAX_PIN_LENGTH 10

class PinManager {
public:
    PinManager(DisplayManager& display);
    void begin();
    
    // Separate methods for device and BLE PIN control
    bool isPinEnabledForDevice();
    bool isPinEnabledForBle();
    bool isPinSet();
    bool requestPin();
    
    // Methods for web server interaction
    void setPin(const String& newPin);
    void setPinEnabledForDevice(bool enabled);
    void setPinEnabledForBle(bool enabled);
    int getPinLength();
    void setPinLength(int newLength);
    void saveConfig();
    void loadPinConfig();
    
    // Legacy method for backward compatibility
    bool isPinEnabled(); // Returns true if either device or BLE PIN is enabled
    void setEnabled(bool enabled); // Sets both device and BLE to the same value

private:
    DisplayManager& displayManager;
    int currentPinLength = DEFAULT_PIN_LENGTH;
    bool enabledForDevice = false;
    bool enabledForBle = false;
    String pinHash = "";

    void savePinConfig();
    bool checkPin(const String& pin);
    void drawPinScreen();
    void updatePinScreen(int currentPosition, int currentDigit, const String& enteredPin);
};

#endif // PIN_MANAGER_H
