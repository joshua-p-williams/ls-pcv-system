#pragma once

#include <cstdint>

namespace ls_pcv {
namespace measurement {

/// Result of one bounded I2C register transaction.
///
/// Keeping bus failures separate from ADS1115 conversion results lets the
/// acquisition layer tell “the ADC did not answer” from “the ADC answered but
/// the analog input is not usable.” The Wire implementation maps platform
/// return codes into these portable outcomes; host tests use a fake bus.
enum class I2cTransactionResult : std::uint8_t {
  kSuccess,
  kNotStarted,
  kAddressNack,
  kWriteFailure,
  kShortRead,
  kTimeout
};

/// Minimal register-oriented seam between the ADC driver and Arduino Wire.
///
/// This interface deliberately does not expose arbitrary I2C operations. The
/// ADS1115 adapter needs only bus startup and 16-bit register reads/writes.
/// The interface also makes register behavior deterministic in native tests.
class I2cRegisterBus {
 public:
  virtual ~I2cRegisterBus() = default;

  /// Start the bus with an implementation-defined finite transaction timeout.
  virtual I2cTransactionResult begin() = 0;

  /// Write a 16-bit register value in device-required most-significant order.
  virtual I2cTransactionResult writeRegister(std::uint8_t address,
                                             std::uint8_t reg,
                                             std::uint16_t value) = 0;

  /// Read one 16-bit register value, changing output only after a full read.
  virtual I2cTransactionResult readRegister(std::uint8_t address,
                                            std::uint8_t reg,
                                            std::uint16_t& value) = 0;
};

}  // namespace measurement
}  // namespace ls_pcv
