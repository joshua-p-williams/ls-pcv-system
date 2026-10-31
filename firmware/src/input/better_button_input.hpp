#pragma once

#include <Arduino.h>
#include <BetterButton.h>

#include "gauge_core/gauge_types.hpp"

namespace ls_pcv {
namespace input {

/// Adapts the BetterButton event codes to the hardware-independent M01 gesture type.
class BetterButtonInput {
 public:
  /// Create a button adapter for a board-selected GPIO.
  /// @param gpioPin GPIO connected to the normally-open switch-to-ground circuit.
  explicit BetterButtonInput(std::uint8_t gpioPin);

  /// Configure BetterButton's internal pull-up and initialize its state.
  /// The library API has no hardware-presence result; an open/missing switch simply
  /// produces no gesture, and the rest of firmware startup continues.
  void begin();

  /// Poll once without waiting for an event.
  /// @param gesture Receives the translated gesture when one is available.
  /// @return True when this poll produced one click, double-click, or long-press.
  bool poll(gauge_core::Gesture& gesture);

 private:
  BetterButton button_;
};

}  // namespace input
}  // namespace ls_pcv
