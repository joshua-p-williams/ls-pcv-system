#include "arduino_wire_bus.hpp"

namespace ls_pcv {
namespace board {
namespace n16r8 {

/// Store this board's documented I2C configuration; Wire starts on begin().
ArduinoWireBus::ArduinoWireBus(TwoWire& wire, int sdaPin, int sclPin,
                               std::uint32_t frequencyHz,
                               std::uint16_t transactionTimeoutMs)
    : wire_(wire), sdaPin_(sdaPin), sclPin_(sclPin),
      frequencyHz_(frequencyHz),
      transactionTimeoutMs_(transactionTimeoutMs) {}

/// Start the ESP32 I2C peripheral with an explicit finite transaction timeout.
measurement::I2cTransactionResult ArduinoWireBus::begin() {
  if (started_) return measurement::I2cTransactionResult::kSuccess;
  started_ = wire_.begin(sdaPin_, sclPin_, frequencyHz_);
  if (!started_) return measurement::I2cTransactionResult::kNotStarted;
  wire_.setTimeOut(transactionTimeoutMs_);
  return measurement::I2cTransactionResult::kSuccess;
}

/// Write register pointer and a 16-bit value in ADS1115 big-endian order.
measurement::I2cTransactionResult ArduinoWireBus::writeRegister(
    std::uint8_t address, std::uint8_t reg, std::uint16_t value) {
  if (!started_) return measurement::I2cTransactionResult::kNotStarted;

  wire_.beginTransmission(address);
  if (wire_.write(reg) != 1 ||
      wire_.write(static_cast<std::uint8_t>(value >> 8)) != 1 ||
      wire_.write(static_cast<std::uint8_t>(value & 0xFFu)) != 1) {
    return measurement::I2cTransactionResult::kWriteFailure;
  }
  return mapWireStatus(wire_.endTransmission(true));
}

/// Select a register and require both response bytes before publishing a value.
measurement::I2cTransactionResult ArduinoWireBus::readRegister(
    std::uint8_t address, std::uint8_t reg, std::uint16_t& value) {
  if (!started_) return measurement::I2cTransactionResult::kNotStarted;

  wire_.beginTransmission(address);
  if (wire_.write(reg) != 1) {
    return measurement::I2cTransactionResult::kWriteFailure;
  }
  const measurement::I2cTransactionResult selectResult =
      mapWireStatus(wire_.endTransmission(false));
  if (selectResult != measurement::I2cTransactionResult::kSuccess) {
    return selectResult;
  }

  const std::uint8_t received = wire_.requestFrom(address, 2);
  if (received != 2 || wire_.available() < 2) {
    while (wire_.available() > 0) (void)wire_.read();
    return measurement::I2cTransactionResult::kShortRead;
  }

  const int highByte = wire_.read();
  const int lowByte = wire_.read();
  if (highByte < 0 || lowByte < 0) {
    return measurement::I2cTransactionResult::kShortRead;
  }
  const std::uint16_t high = static_cast<std::uint8_t>(highByte);
  const std::uint16_t low = static_cast<std::uint8_t>(lowByte);
  value = static_cast<std::uint16_t>((high << 8) | low);
  return measurement::I2cTransactionResult::kSuccess;
}

/// Convert Espressif Wire endTransmission codes into explicit bus outcomes.
measurement::I2cTransactionResult ArduinoWireBus::mapWireStatus(
    std::uint8_t status) const {
  // Arduino-ESP32 Wire documents 2 as address NACK and 5 as timeout. Other
  // nonzero results indicate a transaction/write failure.
  if (status == 0) return measurement::I2cTransactionResult::kSuccess;
  if (status == 2) return measurement::I2cTransactionResult::kAddressNack;
  if (status == 5) return measurement::I2cTransactionResult::kTimeout;
  return measurement::I2cTransactionResult::kWriteFailure;
}

}  // namespace n16r8
}  // namespace board
}  // namespace ls_pcv
