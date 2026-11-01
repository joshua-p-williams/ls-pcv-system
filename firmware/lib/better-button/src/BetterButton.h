#ifndef BetterButton_h
#define BetterButton_h
#include "Arduino.h"

// The switch is wired from GPIO to ground, so INPUT_PULLUP makes an idle/open
// switch HIGH and a pressed switch LOW.
#define BUTTON_RELEASED 0x1
#define BUTTON_PRESSED 0x0

// Event values returned by input(); zero means that no complete gesture exists.
#define BUTTON_CLICKED 1
#define BUTTON_DOUBLE_CLICKED 2
#define BUTTON_LONG_PRESSED 3

// Timing values are milliseconds. input() must be polled regularly for these
// elapsed-time checks to observe press, release, and debounce transitions.
#define LONG_PRESS_DELAY 2000
#define DOUBLE_CLICK_MONITOR_DELAY 500
#define DEBOUNCE_DELAY 60

class BetterButton
{
public:
    /// Store the GPIO only; begin() configures Arduino's pin mode.
    BetterButton(uint8_t pin);

    /// Enable the internal pull-up and clear the gesture recognizer state.
    void begin();

    /// Forget the active gesture/timing sequence without changing pin mode.
    void reset();

    /// Sample the pin and return one event code, or zero when no event is ready.
    /// This is a polling API, not an interrupt handler or blocking wait.
    int input();

private:
    /// Physical input and the debounced state used for gesture recognition.
    uint8_t _pin;

    bool _lastDebouncedState;
    bool _debouncing;
    uint32_t _debounceUntil;

    /// Press/release timestamps and count for the current click sequence.
    uint32_t _pressedAt;
    uint32_t _releasedAt;
    int _clickCount;
    /// Timestamp captured once per input() call so one poll uses a consistent time.
    uint32_t _cachedMillis;

    /// Gesture predicates evaluated after each pin sample.
    bool clicked();
    bool longPressed();
    bool doubleClicked();
    bool monitoring();
    /// Convert a debounced HIGH/LOW transition into gesture timing state.
    void stateChanged(bool state);

    /// Apply the debounce window and forward only stable pin transitions.
    void read();
};

#endif
