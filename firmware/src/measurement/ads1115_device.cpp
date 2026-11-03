#include "ads1115_device.hpp"

#include <cstdint>

namespace ls_pcv {
namespace measurement {
namespace {

/// ADS1115 conversion step at the selected +/-6.144 V PGA setting.
/// 6.144 V / 32768 positive codes = 187.5 microvolts per code.
constexpr std::int64_t kMicrovoltsNumerator = 6144000;
constexpr std::int64_t kPositiveCodeCount = 32768;

/// Round a signed rational to nearest integer, with exact halves away from zero.
std::int32_t roundedSignedRatio(std::int64_t numerator,
                               std::int64_t denominator) {
  if (numerator >= 0) {
    return static_cast<std::int32_t>(
        (numerator + denominator / 2) / denominator);
  }
  return static_cast<std::int32_t>(
      -((-numerator + denominator / 2) / denominator));
}

}  // namespace

/// Bind this driver to the single register bus and selected 7-bit address.
Ads1115Device::Ads1115Device(I2cRegisterBus& bus, std::uint8_t address)
    : bus_(bus), address_(address) {}

/// Read a known register to check that an I2C target responds at this address.
Ads1115Result Ads1115Device::initialize() {
  std::uint16_t ignoredConfig = 0;
  const Ads1115Result result =
      mapBusResult(bus_.readRegister(address_, kConfigurationRegister,
                                     ignoredConfig));
  initialized_ = result == Ads1115Result::kSuccess;
  return result;
}

/// Write the documented A0, single-shot, +/-6.144 V, 128 SPS configuration.
Ads1115Result Ads1115Device::startSingleShotA0() {
  if (!initialized_) return Ads1115Result::kNotInitialized;
  return mapBusResult(bus_.writeRegister(
      address_, kConfigurationRegister, kSingleShotA0_6V144_128Sps));
}

/// Check the OS (operational status) bit without waiting for conversion.
Ads1115Result Ads1115Device::conversionReady(bool& ready) {
  if (!initialized_) return Ads1115Result::kNotInitialized;
  std::uint16_t config = 0;
  const Ads1115Result result =
      mapBusResult(bus_.readRegister(address_, kConfigurationRegister, config));
  if (result != Ads1115Result::kSuccess) return result;
  ready = (config & 0x8000u) != 0;
  return ready ? Ads1115Result::kSuccess : Ads1115Result::kNotReady;
}

/// Fetch the signed two's-complement conversion result after OS becomes one.
Ads1115Result Ads1115Device::readConversion(std::int16_t& code) {
  if (!initialized_) return Ads1115Result::kNotInitialized;
  std::uint16_t word = 0;
  const Ads1115Result result =
      mapBusResult(bus_.readRegister(address_, kConversionRegister, word));
  if (result != Ads1115Result::kSuccess) return result;

  // Convert two's-complement explicitly instead of relying on an
  // implementation-defined unsigned-to-signed narrowing conversion.
  const std::int32_t signedValue = (word & 0x8000u) != 0
      ? static_cast<std::int32_t>(word) - 65536
      : static_cast<std::int32_t>(word);
  code = static_cast<std::int16_t>(signedValue);
  return Ads1115Result::kSuccess;
}

/// Scale a signed code using the selected PGA's exact 187.5 uV/code step.
std::int32_t Ads1115Device::codeToMicrovolts(std::int16_t code) {
  return roundedSignedRatio(
      static_cast<std::int64_t>(code) * kMicrovoltsNumerator,
      kPositiveCodeCount);
}

/// Preserve timeout separately because a timed-out bus is diagnostic evidence.
Ads1115Result Ads1115Device::mapBusResult(
    I2cTransactionResult result) const {
  switch (result) {
    case I2cTransactionResult::kSuccess: return Ads1115Result::kSuccess;
    case I2cTransactionResult::kTimeout: return Ads1115Result::kI2cTimeout;
    case I2cTransactionResult::kNotStarted:
    case I2cTransactionResult::kAddressNack:
    case I2cTransactionResult::kWriteFailure:
    case I2cTransactionResult::kShortRead:
      return Ads1115Result::kI2cError;
  }
  return Ads1115Result::kI2cError;
}

}  // namespace measurement
}  // namespace ls_pcv
