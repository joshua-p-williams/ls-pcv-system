#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "measurement/i2c_register_bus.hpp"

namespace ls_pcv {
namespace board {
namespace n16r8 {

/// Arduino Wire adapter for the small project-owned register interface.
///
/// It is kept in the board layer because Wire, pins, clock rate, and timeout
/// are platform details. Native tests compile the ADS1115 state machine without
/// Arduino headers and substitute a deterministic fake register bus.
class ArduinoWireBus final : public measurement::I2cRegisterBus {
 public:
  ArduinoWireBus(TwoWire& wire, int sdaPin, int sclPin,
                 std::uint32_t frequencyHz,
                 std::uint16_t transactionTimeoutMs);

  measurement::I2cTransactionResult begin() override;
  measurement::I2cTransactionResult writeRegister(
      std::uint8_t address, std::uint8_t reg,
      std::uint16_t value) override;
  measurement::I2cTransactionResult readRegister(
      std::uint8_t address, std::uint8_t reg,
      std::uint16_t& value) override;

 private:
  measurement::I2cTransactionResult mapWireStatus(std::uint8_t status) const;

  TwoWire& wire_;
  int sdaPin_;
  int sclPin_;
  std::uint32_t frequencyHz_;
  std::uint16_t transactionTimeoutMs_;
  bool started_ = false;
};

}  // namespace n16r8
}  // namespace board
}  // namespace ls_pcv
