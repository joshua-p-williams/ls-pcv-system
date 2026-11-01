#include "BetterButton.h"
#include <Arduino.h>

/// Keep the passed GPIO for later pinMode()/digitalRead() calls.
BetterButton::BetterButton(uint8_t pin)
{
    _pin = pin;
}

/// INPUT_PULLUP provides a defined idle HIGH level without an external resistor.
/// The physical switch closes to ground, making a press read LOW.
void BetterButton::begin()
{
    pinMode(_pin, INPUT_PULLUP);
    reset();
}

/// Clear gesture timing and debounce history while preserving the configured pin.
void BetterButton::reset()
{
    _lastDebouncedState = digitalRead(_pin);
    _debouncing = false;
    _debounceUntil = 0;

    _pressedAt = 0;
    _releasedAt = 0;
    _clickCount = 0;
}

/// One nonblocking poll: update time, sample/debounce, then emit at most one event.
/// A returned event is consumed by reset(), so callers should handle it exactly once.
int BetterButton::input()
{
    _cachedMillis = millis();
    int buttonInput = 0;

    read();

    if (_debouncing == false && monitoring() == false)
    {

        if (clicked())
        {
            buttonInput = (doubleClicked() ? BUTTON_DOUBLE_CLICKED : BUTTON_CLICKED);
        }
        else if (longPressed())
        {
            buttonInput = BUTTON_LONG_PRESSED;
        }
    }

    // If we are returning input, consider it handled so reset
    if (buttonInput > 0)
    {
        reset();
    }

    return buttonInput;
}

/// A click is complete only after both a press and its debounced release were seen.
bool BetterButton::clicked()
{
    return _pressedAt > 0 && _releasedAt > 0;
}

/// Measure the press duration using release time when available, otherwise now.
/// Unsigned subtraction keeps elapsed-time behavior safe across millis() rollover.
bool BetterButton::longPressed()
{
    bool isLong = false;

    // We must have captured a "start press", to even consider a long press
    if (_pressedAt > 0)
    {
        // If they haven't let go of the button yet, use the current time
        uint32_t endTime = (_releasedAt > 0 ? _releasedAt : _cachedMillis);

        isLong = ((endTime - _pressedAt) > LONG_PRESS_DELAY);
    }

    return isLong;
}

/// The same press sequence has observed more than one debounced press edge.
bool BetterButton::doubleClicked()
{
    return _pressedAt > 0 && _clickCount > 1;
}

/// Delay final click classification until the double-click window has elapsed.
/// In this implementation the window begins at the initial press timestamp.
bool BetterButton::monitoring()
{
    return (_pressedAt > 0 && (_cachedMillis - _pressedAt) <= DOUBLE_CLICK_MONITOR_DELAY);
}

/// Update the active gesture sequence after a debounced press/release transition.
void BetterButton::stateChanged(bool state)
{

    if (state == BUTTON_PRESSED)
    {
        // If we are still within the double click timeframe
        if (monitoring())
        {
            // We're looking at a double click
            _clickCount++;
            _releasedAt = 0;
        }
        // We are starting a new input sequence now
        else
        {
            _pressedAt = _cachedMillis;
            _releasedAt = 0;
            _clickCount = 1;
        }
    }
    // BUTTON_RELEASED only if it is in a pressed state, otherwise ignore
    else if (_pressedAt > 0)
    {
        _releasedAt = _cachedMillis;
    }
}

/// Read the raw pin only outside an active debounce window. A first edge starts
/// the window; a later stable edge is accepted as the new debounced state.
void BetterButton::read()
{
    // Don't do anything if we are in the middle of a debounce
    if (_debouncing == false || _cachedMillis >= _debounceUntil)
    {
        bool currentState = digitalRead(_pin);

        // If there has been a state change
        if (currentState != _lastDebouncedState)
        {

            // If we were currently debouncing, we can now stop
            if (_debouncing)
            {
                _debouncing = false;
                _lastDebouncedState = currentState;
                stateChanged(currentState);
            }
            // Start debouncing now
            else
            {
                _debouncing = true;
                _debounceUntil = _cachedMillis + DEBOUNCE_DELAY;
            }
        }
    }
}
