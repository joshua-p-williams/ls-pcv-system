#include "better_button_input.hpp"

namespace ls_pcv {
namespace input {

BetterButtonInput::BetterButtonInput(std::uint8_t gpioPin) : button_(gpioPin) {}

/// Delegate GPIO initialization to BetterButton, which selects INPUT_PULLUP.
void BetterButtonInput::begin() {
  button_.begin();
}

/// Convert library-specific integer event codes into the shared application enum.
/// The rest of the firmware therefore does not depend on BetterButton's macros.
bool BetterButtonInput::poll(gauge_core::Gesture& gesture) {
  // BetterButton::input() is a nonblocking poll. Translate only known event codes;
  // an idle poll and any unexpected code leave application state untouched.
  switch (button_.input()) {
    case BUTTON_CLICKED:
      gesture = gauge_core::Gesture::kClick;
      return true;
    case BUTTON_DOUBLE_CLICKED:
      gesture = gauge_core::Gesture::kDoubleClick;
      return true;
    case BUTTON_LONG_PRESSED:
      gesture = gauge_core::Gesture::kLongPress;
      return true;
    default:
      return false;
  }
}

}  // namespace input
}  // namespace ls_pcv
