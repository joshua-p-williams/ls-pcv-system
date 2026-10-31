#ifndef BetterButton_h
#define BetterButton_h
#include "Arduino.h"

#define BUTTON_RELEASED 0x1
#define BUTTON_PRESSED 0x0

#define BUTTON_CLICKED 1
#define BUTTON_DOUBLE_CLICKED 2
#define BUTTON_LONG_PRESSED 3

#define LONG_PRESS_DELAY 2000
#define DOUBLE_CLICK_MONITOR_DELAY 500
#define DEBOUNCE_DELAY 60

class BetterButton
{
public:
    BetterButton(uint8_t pin);
    void begin();
    void reset();
    int input();

private:
    uint8_t _pin;

    bool _lastDebouncedState;
    bool _debouncing;
    uint32_t _debounceUntil;

    uint32_t _pressedAt;
    uint32_t _releasedAt;
    int _clickCount;
    uint32_t _cachedMillis;

    bool clicked();
    bool longPressed();
    bool doubleClicked();
    bool monitoring();
    void stateChanged(bool state);
    void read();
};

#endif