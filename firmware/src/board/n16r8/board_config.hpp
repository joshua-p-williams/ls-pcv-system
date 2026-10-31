#pragma once

#include <Arduino.h>

namespace ls_pcv {
namespace board {
namespace n16r8 {

/// W01 Q14 proposed N16R8 multifunction-button input.
constexpr std::uint8_t kButtonPin = 6;

/// W01 N16R8 SPI display connections for the GC9A01 module.
constexpr std::uint8_t kDisplayDcPin = 4;
constexpr std::uint8_t kDisplayResetPin = 5;
constexpr std::uint8_t kDisplayChipSelectPin = 10;
constexpr std::uint8_t kDisplayMosiPin = 11;
constexpr std::uint8_t kDisplayClockPin = 12;
constexpr std::uint16_t kDisplayWidth = 240;
constexpr std::uint16_t kDisplayHeight = 240;

}  // namespace n16r8
}  // namespace board
}  // namespace ls_pcv
