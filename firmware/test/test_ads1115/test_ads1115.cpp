#include <unity.h>

#include "gauge_core/gauge_types.hpp"
#include "measurement/ads1115_device.hpp"
#include "measurement/ads1115_pressure_source.hpp"

using ls_pcv::measurement::Ads1115Device;
using ls_pcv::measurement::Ads1115PressureSource;
using ls_pcv::measurement::Ads1115Result;
using ls_pcv::measurement::I2cRegisterBus;
using ls_pcv::measurement::I2cTransactionResult;

namespace {

/// Scriptable register model: tests control readiness, data, and bus failures.
class FakeRegisterBus final : public I2cRegisterBus {
 public:
  I2cTransactionResult beginResult = I2cTransactionResult::kSuccess;
  I2cTransactionResult nextReadResult = I2cTransactionResult::kSuccess;
  I2cTransactionResult nextWriteResult = I2cTransactionResult::kSuccess;
  std::uint16_t configWord = 0;
  std::uint16_t conversionWord = 0;
  std::uint8_t lastAddress = 0;
  std::uint8_t lastRegister = 0;
  std::uint16_t lastWrittenValue = 0;
  unsigned beginCalls = 0;
  unsigned readCalls = 0;
  unsigned writeCalls = 0;

  I2cTransactionResult begin() override {
    ++beginCalls;
    return beginResult;
  }

  I2cTransactionResult writeRegister(std::uint8_t address, std::uint8_t reg,
                                     std::uint16_t value) override {
    ++writeCalls;
    lastAddress = address;
    lastRegister = reg;
    lastWrittenValue = value;
    if (nextWriteResult != I2cTransactionResult::kSuccess) {
      const I2cTransactionResult result = nextWriteResult;
      nextWriteResult = I2cTransactionResult::kSuccess;
      return result;
    }
    if (reg == Ads1115Device::kConfigurationRegister) {
      // OS is read-only status after the start command and clears while busy.
      configWord = value & 0x7FFFu;
    }
    return I2cTransactionResult::kSuccess;
  }

  I2cTransactionResult readRegister(std::uint8_t address, std::uint8_t reg,
                                    std::uint16_t& value) override {
    ++readCalls;
    lastAddress = address;
    lastRegister = reg;
    if (nextReadResult != I2cTransactionResult::kSuccess) {
      const I2cTransactionResult result = nextReadResult;
      nextReadResult = I2cTransactionResult::kSuccess;
      return result;
    }
    if (reg == Ads1115Device::kConfigurationRegister) {
      value = configWord;
    } else if (reg == Ads1115Device::kConversionRegister) {
      value = conversionWord;
    } else {
      return I2cTransactionResult::kWriteFailure;
    }
    return I2cTransactionResult::kSuccess;
  }
};

void test_ads1115_initializes_and_writes_selected_configuration() {
  FakeRegisterBus bus;
  Ads1115Device device(bus, 0x48);
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.initialize()));
  TEST_ASSERT_EQUAL_UINT8(0x48, bus.lastAddress);
  TEST_ASSERT_EQUAL_UINT8(Ads1115Device::kConfigurationRegister,
                          bus.lastRegister);
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.startSingleShotA0()));
  TEST_ASSERT_EQUAL_UINT16(Ads1115Device::kSingleShotA0_6V144_128Sps,
                           bus.lastWrittenValue);
  TEST_ASSERT_EQUAL_UINT8(Ads1115Device::kConfigurationRegister,
                          bus.lastRegister);
}

void test_ads1115_requires_initialization_and_checks_bus_results() {
  FakeRegisterBus bus;
  Ads1115Device device(bus);
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kNotInitialized),
                    static_cast<int>(device.startSingleShotA0()));

  bus.nextReadResult = I2cTransactionResult::kAddressNack;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kI2cError),
                    static_cast<int>(device.initialize()));
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kNotInitialized),
                    static_cast<int>(device.startSingleShotA0()));

  bus.nextReadResult = I2cTransactionResult::kTimeout;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kI2cTimeout),
                    static_cast<int>(device.initialize()));
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.initialize()));
  bus.nextWriteResult = I2cTransactionResult::kWriteFailure;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kI2cError),
                    static_cast<int>(device.startSingleShotA0()));
}

void test_ads1115_ready_poll_and_conversion_read_are_distinct() {
  FakeRegisterBus bus;
  Ads1115Device device(bus);
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.initialize()));
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.startSingleShotA0()));

  bool ready = true;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kNotReady),
                    static_cast<int>(device.conversionReady(ready)));
  TEST_ASSERT_FALSE(ready);

  bus.configWord |= 0x8000u;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.conversionReady(ready)));
  TEST_ASSERT_TRUE(ready);

  // The ADC register uses signed 16-bit two's-complement values.
  bus.conversionWord = 0xF000u;
  std::int16_t code = 0;
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kSuccess),
                    static_cast<int>(device.readConversion(code)));
  TEST_ASSERT_EQUAL_INT(-4096, code);
}

void test_ads1115_signed_code_scaling_uses_187_point_5_microvolts() {
  TEST_ASSERT_EQUAL_INT32(0, Ads1115Device::codeToMicrovolts(0));
  TEST_ASSERT_EQUAL_INT32(188, Ads1115Device::codeToMicrovolts(1));
  TEST_ASSERT_EQUAL_INT32(-188, Ads1115Device::codeToMicrovolts(-1));
  TEST_ASSERT_EQUAL_INT32(1500000, Ads1115Device::codeToMicrovolts(8000));
  TEST_ASSERT_EQUAL_INT32(6143813,
                          Ads1115Device::codeToMicrovolts(32767));
  TEST_ASSERT_EQUAL_INT32(-6144000,
                          Ads1115Device::codeToMicrovolts(-32768));
}

void test_pressure_source_records_voltage_and_pressure_before_snapshot() {
  FakeRegisterBus bus;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_FALSE(source.poll(100));  // Initialize only; one I2C probe.
  TEST_ASSERT_FALSE(source.snapshot().hasPressure);
  TEST_ASSERT_EQUAL_UINT32(1, bus.readCalls);
  TEST_ASSERT_EQUAL_UINT32(0, bus.writeCalls);
  TEST_ASSERT_FALSE(source.poll(101));  // Start conversion on a separate turn.
  TEST_ASSERT_EQUAL_UINT32(1, bus.writeCalls);

  bus.configWord |= 0x8000u;
  bus.conversionWord = 8000u;  // 1.5 V provisional atmospheric reference.
  TEST_ASSERT_FALSE(source.poll(108));  // Observe readiness, do not read yet.
  TEST_ASSERT_EQUAL_UINT32(2, bus.readCalls);
  TEST_ASSERT_TRUE(source.poll(109));   // Read/process conversion separately.
  TEST_ASSERT_EQUAL_UINT32(3, bus.readCalls);
  const gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementSource::kLive),
                    static_cast<int>(snapshot.source));
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kValid),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_TRUE(snapshot.hasRawAdcCode);
  TEST_ASSERT_EQUAL_INT(8000, snapshot.rawAdcCode);
  TEST_ASSERT_TRUE(snapshot.hasSignalVoltage);
  TEST_ASSERT_EQUAL_UINT32(1500000, snapshot.signalVoltageMicrovolts);
  TEST_ASSERT_TRUE(snapshot.hasPressure);
  TEST_ASSERT_EQUAL_INT(0, snapshot.pressure);
  TEST_ASSERT_EQUAL_INT(0, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(0, snapshot.maximumRecorded);
}

void test_pressure_source_reports_initialization_failure_and_retries_bounded() {
  FakeRegisterBus bus;
  bus.beginResult = I2cTransactionResult::kTimeout;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_TRUE(source.poll(10));
  gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kI2cError),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_FALSE(snapshot.hasPressure);
  TEST_ASSERT_EQUAL_UINT32(1, bus.beginCalls);

  TEST_ASSERT_FALSE(source.poll(1009));
  TEST_ASSERT_EQUAL_UINT32(1, bus.beginCalls);
  bus.beginResult = I2cTransactionResult::kSuccess;
  TEST_ASSERT_FALSE(source.poll(1010));
  TEST_ASSERT_EQUAL_UINT32(2, bus.beginCalls);
}

void test_pressure_source_bounds_conversion_wait_and_exposes_fault() {
  FakeRegisterBus bus;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_FALSE(source.poll(0));
  TEST_ASSERT_FALSE(source.poll(1));
  TEST_ASSERT_FALSE(source.poll(25));
  TEST_ASSERT_TRUE(source.poll(26));
  const gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kAdcError),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_FALSE(snapshot.hasPressure);
  TEST_ASSERT_EQUAL(static_cast<int>(Ads1115Result::kConversionTimeout),
                    static_cast<int>(source.lastDeviceResult()));
}

void test_pressure_source_rejects_negative_single_ended_code() {
  FakeRegisterBus bus;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_FALSE(source.poll(0));
  TEST_ASSERT_FALSE(source.poll(1));
  bus.configWord |= 0x8000u;
  bus.conversionWord = 0xFFFFu;  // -1 code: below ground is not valid A0 input.
  TEST_ASSERT_FALSE(source.poll(8));
  TEST_ASSERT_TRUE(source.poll(9));
  const gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kAdcError),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_TRUE(snapshot.hasRawAdcCode);
  TEST_ASSERT_EQUAL_INT(-1, snapshot.rawAdcCode);
  TEST_ASSERT_FALSE(snapshot.hasSignalVoltage);
  TEST_ASSERT_FALSE(snapshot.hasPressure);
}

void test_pressure_source_marks_voltage_outside_sensor_profile() {
  FakeRegisterBus bus;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_FALSE(source.poll(0));
  TEST_ASSERT_FALSE(source.poll(1));
  bus.configWord |= 0x8000u;
  bus.conversionWord = 2600u;  // About 0.4875 V, below the provisional 0.5 V.
  TEST_ASSERT_FALSE(source.poll(8));
  TEST_ASSERT_TRUE(source.poll(9));
  const gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kOutOfDomain),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_TRUE(snapshot.hasRawAdcCode);
  TEST_ASSERT_TRUE(snapshot.hasSignalVoltage);
  TEST_ASSERT_FALSE(snapshot.hasPressure);
}

void test_pressure_source_clears_current_pressure_after_i2c_failure() {
  FakeRegisterBus bus;
  Ads1115PressureSource source(bus, 0x48);
  TEST_ASSERT_FALSE(source.poll(0));
  TEST_ASSERT_FALSE(source.poll(1));
  bus.configWord |= 0x8000u;
  bus.conversionWord = 8000u;
  TEST_ASSERT_FALSE(source.poll(8));
  TEST_ASSERT_TRUE(source.poll(9));
  TEST_ASSERT_TRUE(source.snapshot().hasPressure);

  // Start a subsequent conversion, then fail the readiness register read.
  TEST_ASSERT_FALSE(source.poll(10));
  bus.nextReadResult = I2cTransactionResult::kAddressNack;
  TEST_ASSERT_TRUE(source.poll(11));
  const gauge_core::MeasurementSnapshot snapshot = source.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(gauge_core::MeasurementQuality::kI2cError),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_FALSE(snapshot.hasPressure);
}

}  // namespace

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ads1115_initializes_and_writes_selected_configuration);
  RUN_TEST(test_ads1115_requires_initialization_and_checks_bus_results);
  RUN_TEST(test_ads1115_ready_poll_and_conversion_read_are_distinct);
  RUN_TEST(test_ads1115_signed_code_scaling_uses_187_point_5_microvolts);
  RUN_TEST(test_pressure_source_records_voltage_and_pressure_before_snapshot);
  RUN_TEST(test_pressure_source_reports_initialization_failure_and_retries_bounded);
  RUN_TEST(test_pressure_source_bounds_conversion_wait_and_exposes_fault);
  RUN_TEST(test_pressure_source_rejects_negative_single_ended_code);
  RUN_TEST(test_pressure_source_marks_voltage_outside_sensor_profile);
  RUN_TEST(test_pressure_source_clears_current_pressure_after_i2c_failure);
  return UNITY_END();
}
