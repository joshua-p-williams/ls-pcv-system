#pragma once

#include <Arduino.h>
#include <BetterButton.h>

#include "gauge_core/gauge_types.hpp"

namespace ls_pcv {
namespace input {

/// Adapts Arduino GPIO and BetterButton event codes to M01's hardware-independent
/// gesture type. The adapter does not decide what a gesture means to the gauge.
class BetterButtonInput {
 public:
  /// Create a button adapter for a board-selected GPIO.
  /// Construction stores configuration only; call begin() after Arduino startup.
  /// @param gpioPin GPIO connected to the normally-open switch-to-ground circuit.
  explicit BetterButtonInput(std::uint8_t gpioPin);

  /// Configure BetterButton's internal pull-up and initialize its state.
  /// A pull-up makes an open switch read HIGH; the switch connects the input to
  /// ground when pressed, producing LOW. BetterButton handles debounce/gestures.
  /// The library API has no hardware-presence result; an open/missing switch simply
  /// produces no gesture, and the rest of firmware startup continues.
  void begin();

  /// Poll once without waiting for an event. Call frequently because the library
  /// infers press durations and double-click timing from successive observations.
  /// @param gesture Receives the translated gesture when one is available.
  /// @return True when this poll produced one click, double-click, or long-press.
  bool poll(gauge_core::Gesture& gesture);

 private:
  BetterButton button_;
};

}  // namespace input
}  // namespace ls_pcv
