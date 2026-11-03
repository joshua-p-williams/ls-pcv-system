#pragma once

#include <cstdint>

namespace ls_pcv {
namespace board {
namespace n16r8 {

/// W01 Q14 N16R8 multifunction-button input GPIO, connected to ground on press.
constexpr std::uint8_t kButtonPin = 6;

/// W01 Q14 N16R8 I2C mapping through the LV side of the level translator.
/// Pin assignment is the documented bench design; bus operation is verified
/// separately during Phase 3 bring-up.
constexpr std::uint8_t kI2cSdaPin = 8;
constexpr std::uint8_t kI2cSclPin = 9;
constexpr std::uint32_t kI2cFrequencyHz = 100000;
/// W01 ADDR_GND selects the ADS1115's 7-bit 0x48 address candidate.
constexpr std::uint8_t kAds1115Address = 0x48;
/// Bound each ESP32 Wire transaction so a missing module cannot stall a task.
constexpr std::uint16_t kI2cTransactionTimeoutMs = 10;

/// W01 N16R8 SPI display connections for the GC9A01 module. These aliases keep
/// carrier-specific GPIO choices out of the display and gauge logic. They are
/// the current bench mapping, not the future XIAO pin map.
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
