#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "board/n16r8/arduino_wire_bus.hpp"
#include "board/n16r8/board_config.hpp"
#include "measurement/ads1115_pressure_source.hpp"

namespace {

// A one-item queue is a mailbox: the acquisition task can replace an older
// diagnostic snapshot when USB printing is slower than ADC conversions.
// Every conversion is still processed by Ads1115PressureSource before it is
// published; only optional text output may skip intermediate snapshots.
struct BenchVoltageReading {
  gauge_core::MeasurementQuality quality =
      gauge_core::MeasurementQuality::kUnavailable;
  std::uint32_t acquiredAtMs = 0;
  bool hasRawAdcCode = false;
  std::int16_t rawAdcCode = 0;
  bool hasSignalVoltage = false;
  std::uint32_t signalVoltageMicrovolts = 0;
  std::uint32_t sampleSequence = 0;
};

constexpr std::uint32_t kAcquisitionTaskDelayMs = 2;
constexpr std::uint32_t kDiagnosticIntervalMs = 250;
constexpr std::uint32_t kDiagnosticTaskStackBytes = 4096;
constexpr UBaseType_t kAcquisitionTaskPriority = 2;

// These objects compose the actual N16R8 hardware with the tested ADC source.
// Wire, GPIO and timing belong here; pressure rules remain independent of the
// board and can be exercised on a host computer with a fake register bus.
ls_pcv::board::n16r8::ArduinoWireBus i2cBus(
    Wire,
    ls_pcv::board::n16r8::kI2cSdaPin,
    ls_pcv::board::n16r8::kI2cSclPin,
    ls_pcv::board::n16r8::kI2cFrequencyHz,
    ls_pcv::board::n16r8::kI2cTransactionTimeoutMs);
ls_pcv::measurement::Ads1115PressureSource adcSource(
    i2cBus, ls_pcv::board::n16r8::kAds1115Address);
QueueHandle_t snapshotMailbox = nullptr;

/// Name the electrical acquisition result without exposing pressure-domain status.
const char* acquisitionStateName(const BenchVoltageReading& reading) {
  // The shared pressure source also checks a provisional sensor-voltage domain.
  // A bench battery can be outside that domain while still being a perfectly
  // usable A0 test voltage, so any complete raw-code/voltage pair is reported
  // as a successful voltage read regardless of its pressure-domain quality.
  if (reading.hasRawAdcCode && reading.hasSignalVoltage) return "voltage_read";

  const gauge_core::MeasurementQuality quality = reading.quality;
  switch (quality) {
    case gauge_core::MeasurementQuality::kUnavailable: return "unavailable";
    case gauge_core::MeasurementQuality::kStale: return "stale";
    case gauge_core::MeasurementQuality::kAdcError: return "adc_error";
    case gauge_core::MeasurementQuality::kI2cError: return "i2c_error";
    case gauge_core::MeasurementQuality::kValid: return "incomplete_sample";
    case gauge_core::MeasurementQuality::kOutOfDomain: return "out_of_domain";
  }
  return "unknown";
}

/// Own every ADS1115 transaction on one task, away from USB and UI work.
///
/// poll() performs at most one bounded I2C transaction. It does not wait for
/// the ADC conversion interval; task delays let the processor run other work
/// while the converter makes its next result. The mailbox transfers an
/// immutable copy, so the Arduino loop never reads the tracker's mutable state.
void adcAcquisitionTask(void*) {
  std::uint32_t sampleSequence = 0;

  for (;;) {
    if (adcSource.poll(millis())) {
      const gauge_core::MeasurementSnapshot snapshot = adcSource.snapshot();

      // Count only observations that contain a newly read conversion code.
      // Bus errors are published too, but cannot look like fresh samples.
      if (snapshot.hasRawAdcCode) ++sampleSequence;

      // Copy only voltage-acquisition fields into the bench mailbox. The
      // pressure value that the shared source can derive is intentionally not
      // part of this diagnostic interface, so test batteries cannot be
      // presented as crankcase readings by mistake.
      BenchVoltageReading message;
      message.quality = snapshot.quality;
      message.acquiredAtMs = snapshot.acquiredAtMs;
      message.hasRawAdcCode = snapshot.hasRawAdcCode;
      message.rawAdcCode = snapshot.rawAdcCode;
      message.hasSignalVoltage = snapshot.hasSignalVoltage;
      message.signalVoltageMicrovolts = snapshot.signalVoltageMicrovolts;
      message.sampleSequence = sampleSequence;
      (void)xQueueOverwrite(snapshotMailbox, &message);
    }

    vTaskDelay(pdMS_TO_TICKS(kAcquisitionTaskDelayMs));
  }
}

/// Print the latest result at a human-readable rate over optional USB.
/// The ADC task never writes serial data, and setup gives CDC a zero wait limit.
void reportBenchSnapshot(const BenchVoltageReading& message) {
  Serial.printf("M02_ADC state=%s sample=%lu acquired_ms=%lu",
                acquisitionStateName(message),
                static_cast<unsigned long>(message.sampleSequence),
                static_cast<unsigned long>(message.acquiredAtMs));

  if (message.hasRawAdcCode) {
    Serial.printf(" raw_code=%d", static_cast<int>(message.rawAdcCode));
  } else {
    Serial.print(" raw_code=unavailable");
  }

  if (message.hasSignalVoltage) {
    Serial.printf(" a0_uV=%lu",
                  static_cast<unsigned long>(message.signalVoltageMicrovolts));
  } else {
    Serial.print(" a0_uV=unavailable");
  }

  // A battery is an electrical test source, not a pressure calibration.
  Serial.println(" pressure=not-tested");
}

}  // namespace

/// Start optional USB diagnostics, a mailbox, and the independent ADC task.
/// This firmware does not drive a test voltage or require the FTP sensor.
void setup() {
  Serial.begin(115200);
  // CDC writes are best-effort; acquisition must continue without a monitor.
  Serial.setTxTimeoutMs(0);
  Serial.println("LS PCV M02 ADS1115 voltage-acquisition bench build");
  Serial.println(
      "Input=ADS1115 A0; output=raw code and A0 microvolts; pressure is not tested");
  Serial.printf(
      "ADC address=0x%02X config=0x%04X mux=A0-to-GND pga=+/-6.144V "
      "mode=single-shot rate=128SPS; I2C=%luHz timeout=%ums\n",
      static_cast<unsigned>(ls_pcv::board::n16r8::kAds1115Address),
      static_cast<unsigned>(ls_pcv::measurement::Ads1115Device::
                                kSingleShotA0_6V144_128Sps),
      static_cast<unsigned long>(ls_pcv::board::n16r8::kI2cFrequencyHz),
      static_cast<unsigned>(
          ls_pcv::board::n16r8::kI2cTransactionTimeoutMs));

  snapshotMailbox = xQueueCreate(1, sizeof(BenchVoltageReading));
  if (snapshotMailbox == nullptr) {
    Serial.println("M02_ADC startup_error=snapshot_mailbox_unavailable");
    return;
  }

  const BaseType_t taskResult =
      xTaskCreateUniversal(adcAcquisitionTask, "ads1115Acquire",
                           kDiagnosticTaskStackBytes, nullptr,
                           kAcquisitionTaskPriority, nullptr,
                           ARDUINO_RUNNING_CORE);
  if (taskResult != pdPASS) {
    Serial.println("M02_ADC startup_error=acquisition_task_unavailable");
  }
}

/// Emit the newest copied state four times per second at most.
/// The I2C bus and tracker remain owned by adcAcquisitionTask.
void loop() {
  static BenchVoltageReading latestMessage{};
  static bool hasMessage = false;
  static bool hasReported = false;
  static std::uint32_t lastReportMs = 0;

  if (snapshotMailbox != nullptr &&
      xQueueReceive(snapshotMailbox, &latestMessage, 0) == pdTRUE) {
    hasMessage = true;
  }

  const std::uint32_t nowMs = millis();
  if (hasMessage &&
      (!hasReported || nowMs - lastReportMs >= kDiagnosticIntervalMs)) {
    reportBenchSnapshot(latestMessage);
    lastReportMs = nowMs;
    hasReported = true;
  }

  // Yield when there is no report to print instead of spinning continuously.
  vTaskDelay(pdMS_TO_TICKS(5));
}
