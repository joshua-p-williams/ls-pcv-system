#pragma once

#include <cstdint>

#include "i2c_register_bus.hpp"

namespace ls_pcv {
namespace measurement {

/// Outcome of a single ADS1115 operation.
enum class Ads1115Result : std::uint8_t {
  kSuccess,
  kNotInitialized,
  kI2cError,
  kI2cTimeout,
  kNotReady,
  kConversionTimeout,
  kInvalidSingleEndedCode,
  kPressureConversionError
};

/// Small, project-owned ADS1115 register driver for single-ended A0.
///
/// The ADS1115 is a converter, not a pressure sensor: it returns a signed
/// digital code that represents voltage on A0. This adapter owns only the
/// required register protocol. Pressure meaning and history remain in the
/// hardware-independent gauge core, and Arduino Wire is supplied separately
/// through I2cRegisterBus.
class Ads1115Device {
 public:
  static constexpr std::uint8_t kConversionRegister = 0x00;
  static constexpr std::uint8_t kConfigurationRegister = 0x01;

  /// TI ADS1115 config word: OS=start, MUX=A0-to-GND, PGA=+/-6.144 V,
  /// single-shot mode, 128 SPS, comparator disabled. See TI datasheet Config
  /// Register Table 8-3. PGA range is a code scale, not permission for A0 to
  /// exceed the actual ADS1115 ground-to-VDD range.
  static constexpr std::uint16_t kSingleShotA0_6V144_128Sps = 0xC183;

  explicit Ads1115Device(I2cRegisterBus& bus,
                         std::uint8_t address = 0x48);

  /// Probe the configuration register to establish that the selected address
  /// responds. ADS1115 has no identity register, so this is a communication
  /// check, not proof of chip identity.
  Ads1115Result initialize();

  /// Start a new single conversion of A0 relative to ground.
  Ads1115Result startSingleShotA0();

  /// Read the config OS bit; false means conversion is still in progress.
  Ads1115Result conversionReady(bool& ready);

  /// Read the conversion register only after readiness has been confirmed.
  Ads1115Result readConversion(std::int16_t& code);

  /// Convert one signed ADS1115 code to microvolts at the selected gain.
  /// Negative single-ended results are returned with their sign so the caller
  /// can identify an electrically invalid below-ground condition.
  static std::int32_t codeToMicrovolts(std::int16_t code);

 private:
  Ads1115Result mapBusResult(I2cTransactionResult result) const;

  I2cRegisterBus& bus_;
  std::uint8_t address_;
  bool initialized_ = false;
};

}  // namespace measurement
}  // namespace ls_pcv
