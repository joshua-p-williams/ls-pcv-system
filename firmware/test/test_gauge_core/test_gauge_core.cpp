#include <unity.h>

#include "gauge_core/demo_pressure_source.hpp"
#include "gauge_core/gauge_session.hpp"
#include "gauge_core/measurement_tracker.hpp"
#include "gauge_core/m01_interaction.hpp"
#include "gauge_core/pressure_classifier.hpp"
#include "gauge_core/pressure_converter.hpp"
#include "gauge_core/zero_reference_capture.hpp"

using namespace gauge_core;

/// Unity hook run before every test; the hardware-independent core needs no setup.
void setUp() {}
/// Unity hook run after every test; the core owns no external resources to release.
void tearDown() {}

/// Confirm default synthetic span endpoints are accepted and adjacent values rejected.
void test_default_demo_configuration_and_span() {
  const DemoBandConfig config;
  TEST_ASSERT_TRUE(config.isValid());
  TEST_ASSERT_TRUE(config.contains(-140));
  TEST_ASSERT_TRUE(config.contains(45));
  TEST_ASSERT_FALSE(config.contains(-141));
  TEST_ASSERT_FALSE(config.contains(46));
}

/// Exercise each inclusive/exclusive classifier edge, including zero and both extremes.
void test_classifier_threshold_edges() {
  const DemoBandConfig config;
  const PressureClassifier classifier;
  struct Case { PressureTenthsInH2O value; PressureBand band; };
  const Case cases[] = {
      {-140, PressureBand::kDanger}, {-121, PressureBand::kDanger},
      {-120, PressureBand::kWarning}, {-81, PressureBand::kWarning},
      {-80, PressureBand::kNormal}, {-30, PressureBand::kNormal},
      {-29, PressureBand::kWarning}, {0, PressureBand::kWarning},
      {1, PressureBand::kDanger}, {45, PressureBand::kDanger}};
  for (const Case& item : cases) {
    PressureBand actual = PressureBand::kNormal;
    TEST_ASSERT_TRUE(classifier.classify(item.value, config, actual));
    TEST_ASSERT_EQUAL(static_cast<int>(item.band), static_cast<int>(actual));
  }
}

/// Classification uses status limits; display/sensor domains are checked separately.
void test_classifier_separates_status_limits_from_measurement_domain() {
  const PressureClassifier classifier;
  PressureBand result = PressureBand::kNormal;
  TEST_ASSERT_TRUE(classifier.classify(-141, PressureBandThresholds{}, result));
  TEST_ASSERT_EQUAL(static_cast<int>(PressureBand::kDanger),
                    static_cast<int>(result));
  PressureBandThresholds invalid;
  invalid.lowerWarning = -130;
  TEST_ASSERT_FALSE(classifier.classify(0, invalid, result));
  GaugeSession demoSession;
  TEST_ASSERT_FALSE(demoSession.updatePressure(-141));
}

/// Generated values stay in range and classify into the band requested by the source.
void test_demo_values_are_bounded_and_classified_for_requested_band() {
  const DemoBandConfig config;
  const PressureClassifier classifier;
  DemoPressureSource source(12345);
  PressureTenthsInH2O value = 0;
  PressureBand band = PressureBand::kDanger;
  bool sawWarning = false;
  bool sawDanger = false;
  for (int i = 0; i < 300; ++i) {
    TEST_ASSERT_TRUE(source.nextNormal(config, value));
    TEST_ASSERT_TRUE(config.contains(value));
    TEST_ASSERT_TRUE(classifier.classify(value, config, band));
    TEST_ASSERT_EQUAL(static_cast<int>(PressureBand::kNormal), static_cast<int>(band));
  }
  for (int i = 0; i < 500; ++i) {
    TEST_ASSERT_TRUE(source.nextWarningOrDanger(config, value));
    TEST_ASSERT_TRUE(config.contains(value));
    TEST_ASSERT_TRUE(classifier.classify(value, config, band));
    TEST_ASSERT_TRUE(band == PressureBand::kWarning || band == PressureBand::kDanger);
    sawWarning = sawWarning || band == PressureBand::kWarning;
    sawDanger = sawDanger || band == PressureBand::kDanger;
  }
  TEST_ASSERT_TRUE(sawWarning);
  TEST_ASSERT_TRUE(sawDanger);
}

/// Equal seeds produce equal streams, allowing deterministic synthetic tests.
void test_demo_source_is_repeatable_for_equal_seeds() {
  DemoPressureSource first(0x12345678u);
  DemoPressureSource second(0x12345678u);
  const DemoBandConfig config;
  for (int i = 0; i < 30; ++i) {
    PressureTenthsInH2O a = 0;
    PressureTenthsInH2O b = 0;
    if (i % 2 == 0) {
      TEST_ASSERT_TRUE(first.nextNormal(config, a));
      TEST_ASSERT_TRUE(second.nextNormal(config, b));
    } else {
      TEST_ASSERT_TRUE(first.nextWarningOrDanger(config, a));
      TEST_ASSERT_TRUE(second.nextWarningOrDanger(config, b));
    }
    TEST_ASSERT_EQUAL_INT(a, b);
  }
}

/// The source rejects invalid band ordering instead of generating misleading data.
void test_normal_generator_rejects_invalid_configuration() {
  DemoBandConfig invalid;
  invalid.upperDanger = -50;
  DemoPressureSource source;
  PressureTenthsInH2O value = 0;
  TEST_ASSERT_FALSE(source.nextNormal(invalid, value));
  TEST_ASSERT_FALSE(source.nextWarningOrDanger(invalid, value));
}

/// Sampling waits for its first elapsed second and then records on one-second slots.
void test_sampling_starts_after_first_observation_and_uses_elapsed_seconds() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-55));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(100));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(1099));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(1100));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(2099));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(2100));
  GaugeDisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL_UINT(2, snapshot.historyCount);
  TEST_ASSERT_TRUE(snapshot.history[0].valid);
  TEST_ASSERT_TRUE(snapshot.history[1].valid);
  TEST_ASSERT_EQUAL_INT(-55, snapshot.history[0].pressure);
  TEST_ASSERT_EQUAL_INT(-55, snapshot.history[1].pressure);
}

/// Unsigned elapsed-time arithmetic continues to work when millis() wraps to zero.
void test_sampling_handles_millis_rollover() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-50));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0xFFFFFF00u));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0x000002E7u));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(0x000002E8u));
}

/// History begins partial, fills, then retains the latest 30 chronological entries.
void test_history_is_partial_then_rolls_over_oldest_samples() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-70));
  TEST_ASSERT_EQUAL_UINT(0, session.snapshot().historyCount);
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0));
  for (int i = 1; i <= 35; ++i) {
    TEST_ASSERT_TRUE(session.updatePressure(static_cast<PressureTenthsInH2O>(-70 + i)));
    TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(static_cast<std::uint32_t>(i) * 1000u));
  }
  const GaugeDisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL_UINT(30, snapshot.historyCount);
  TEST_ASSERT_TRUE(snapshot.history[0].valid);
  TEST_ASSERT_TRUE(snapshot.history[29].valid);
  TEST_ASSERT_EQUAL_INT(-64, snapshot.history[0].pressure);
  TEST_ASSERT_EQUAL_INT(-35, snapshot.history[29].pressure);
}

/// Tracker marks unobserved catch-up intervals as gaps and caps ring work.
void test_tracker_catches_up_with_gaps_and_caps_work_to_ring_size() {
  MeasurementTracker tracker;
  TEST_ASSERT_EQUAL_UINT(0, tracker.sampleAt(0));
  MeasurementObservation observation;
  observation.source = MeasurementSource::kLive;
  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = -60;
  observation.acquiredAtMs = 1000;
  TEST_ASSERT_TRUE(tracker.record(observation));
  TEST_ASSERT_EQUAL_UINT(4, tracker.sampleAt(4000));
  MeasurementSnapshot snapshot = tracker.snapshot();
  TEST_ASSERT_EQUAL_UINT(4, snapshot.historyCount);
  TEST_ASSERT_TRUE(snapshot.history[0].valid);
  TEST_ASSERT_FALSE(snapshot.history[1].valid);
  TEST_ASSERT_FALSE(snapshot.history[2].valid);
  TEST_ASSERT_FALSE(snapshot.history[3].valid);
  TEST_ASSERT_EQUAL_UINT(30, tracker.sampleAt(100000));
  TEST_ASSERT_EQUAL_UINT(30, tracker.snapshot().historyCount);
}

/// Synthetic M01 state repeats through stalls, while live observations leave gaps.
void test_demo_history_preserves_repeated_state_during_catch_up() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-60));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0));
  TEST_ASSERT_EQUAL_UINT(4, session.sampleAt(4000));
  const GaugeDisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL_UINT(4, snapshot.historyCount);
  for (std::size_t index = 0; index < snapshot.historyCount; ++index) {
    TEST_ASSERT_TRUE(snapshot.history[index].valid);
    TEST_ASSERT_EQUAL_INT(-60, snapshot.history[index].pressure);
  }
}

/// Accepted readings update extrema; an out-of-span reading changes no session state.
void test_extrema_track_accepted_values_and_reject_out_of_span() {
  GaugeSession session;
  TEST_ASSERT_FALSE(session.snapshot().hasExtrema);
  TEST_ASSERT_TRUE(session.updatePressure(-50));
  TEST_ASSERT_TRUE(session.updatePressure(20));
  TEST_ASSERT_TRUE(session.updatePressure(-130));
  TEST_ASSERT_FALSE(session.updatePressure(46));
  const GaugeDisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(MeasurementSource::kSynthetic),
                    static_cast<int>(snapshot.source));
  TEST_ASSERT_EQUAL(static_cast<int>(MeasurementQuality::kValid),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_TRUE(snapshot.hasExtrema);
  TEST_ASSERT_EQUAL_INT(-130, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(20, snapshot.maximumRecorded);
  TEST_ASSERT_EQUAL_INT(-130, snapshot.pressure);
}

/// Gestures alter demo state/pages, while timer sampling—not gestures—adds history.
void test_gestures_change_demo_state_and_pages_without_sampling() {
  DemoPressureSource source(456);
  GaugeSession session;
  M01Interaction interaction(source, session);

  TEST_ASSERT_TRUE(interaction.handle(Gesture::kClick));
  GaugeDisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(PressureBand::kNormal), static_cast<int>(snapshot.band));
  TEST_ASSERT_EQUAL_UINT(0, snapshot.historyCount);

  TEST_ASSERT_TRUE(interaction.handle(Gesture::kDoubleClick));
  snapshot = session.snapshot();
  TEST_ASSERT_TRUE(snapshot.band == PressureBand::kWarning || snapshot.band == PressureBand::kDanger);
  TEST_ASSERT_EQUAL_UINT(0, snapshot.historyCount);
  session.sampleAt(0);
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(1000));
  snapshot = session.snapshot();

  const PressureTenthsInH2O beforePage = snapshot.pressure;
  const PressureTenthsInH2O minimumBeforePage = snapshot.minimumRecorded;
  const PressureTenthsInH2O maximumBeforePage = snapshot.maximumRecorded;
  const std::size_t historyBeforePage = snapshot.historyCount;
  TEST_ASSERT_TRUE(interaction.handle(Gesture::kLongPress));
  TEST_ASSERT_EQUAL(static_cast<int>(Page::kTrend), static_cast<int>(session.snapshot().page));
  TEST_ASSERT_EQUAL_INT(beforePage, session.snapshot().pressure);
  TEST_ASSERT_EQUAL_UINT(historyBeforePage, session.snapshot().historyCount);
  TEST_ASSERT_TRUE(interaction.handle(Gesture::kLongPress));
  TEST_ASSERT_EQUAL(static_cast<int>(Page::kMinMax), static_cast<int>(session.snapshot().page));
  TEST_ASSERT_TRUE(interaction.handle(Gesture::kLongPress));
  snapshot = session.snapshot();
  TEST_ASSERT_EQUAL(static_cast<int>(Page::kGauge), static_cast<int>(snapshot.page));
  TEST_ASSERT_EQUAL_INT(beforePage, snapshot.pressure);
  TEST_ASSERT_EQUAL_INT(minimumBeforePage, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(maximumBeforePage, snapshot.maximumRecorded);
}

/// Verify the provisional family anchors, branch polarity, and asymmetric slopes.
void test_pressure_converter_family_anchors_and_sign() {
  const PressureConverter converter;
  const PressureCalibrationProfile profile;
  PressureConversionResult result = converter.convert(1500000, profile);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kValid),
                    static_cast<int>(result.status));
  TEST_ASSERT_EQUAL_INT(0, result.pressure);

  result = converter.convert(500000, profile);
  TEST_ASSERT_EQUAL_INT(45, result.pressure);  // +4.5 inH2O reference point.

  result = converter.convert(4500000, profile);
  TEST_ASSERT_EQUAL_INT(-140, result.pressure);  // -14.0 inH2O reference point.

  result = converter.convert(2500000, profile);
  TEST_ASSERT_EQUAL_INT(-47, result.pressure);  // 14/3 inH2O/V, rounded to tenths.
}

/// Reject extrapolation and invalid slope/domain configuration; round half upward.
void test_pressure_converter_rounding_and_invalid_inputs() {
  const PressureConverter converter;
  const PressureCalibrationProfile profile;
  PressureConversionResult result = converter.convert(1400000, profile);
  TEST_ASSERT_EQUAL_INT(5, result.pressure);  // 0.45 inH2O rounds to 0.5.

  result = converter.convert(499999, profile);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kOutOfDomain),
                    static_cast<int>(result.status));
  result = converter.convert(4500001, profile);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kOutOfDomain),
                    static_cast<int>(result.status));

  PressureCalibrationProfile invalid = profile;
  invalid.zeroSignalMicrovolts = 4600000;
  result = converter.convert(1500000, invalid);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kInvalidProfile),
                    static_cast<int>(result.status));
  invalid = profile;
  invalid.vacuumTenthsDenominator = 0;
  result = converter.convert(1500000, invalid);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kInvalidProfile),
                    static_cast<int>(result.status));

  // int16_t can represent -32768, but its positive range stops at +32767.
  PressureCalibrationProfile signedMinimum = profile;
  signedMinimum.vacuumTenthsNumerator = 32768;
  result = converter.convert(4500000, signedMinimum);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kValid),
                    static_cast<int>(result.status));
  TEST_ASSERT_EQUAL_INT(-32768, result.pressure);

  PressureCalibrationProfile positiveOverflow = profile;
  positiveOverflow.positiveTenthsNumerator = 32768;
  result = converter.convert(500000, positiveOverflow);
  TEST_ASSERT_EQUAL(static_cast<int>(PressureConversionStatus::kPressureOverflow),
                    static_cast<int>(result.status));
}

/// Invalid ADC states suppress current pressure and never alter valid extrema.
void test_measurement_tracker_invalid_states_preserve_valid_extrema() {
  MeasurementTracker tracker;
  MeasurementObservation observation;
  observation.source = MeasurementSource::kLive;
  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = -50;
  observation.acquiredAtMs = 100;
  TEST_ASSERT_TRUE(tracker.record(observation));
  observation.pressure = 20;
  observation.acquiredAtMs = 200;
  TEST_ASSERT_TRUE(tracker.record(observation));

  observation.quality = MeasurementQuality::kI2cError;
  observation.hasPressure = false;
  observation.hasRawAdcCode = false;
  observation.hasSignalVoltage = false;
  observation.acquiredAtMs = 300;
  TEST_ASSERT_TRUE(tracker.record(observation));
  MeasurementSnapshot snapshot = tracker.snapshot();
  TEST_ASSERT_FALSE(snapshot.hasPressure);
  TEST_ASSERT_EQUAL(static_cast<int>(MeasurementQuality::kI2cError),
                    static_cast<int>(snapshot.quality));
  TEST_ASSERT_TRUE(snapshot.hasExtrema);
  TEST_ASSERT_EQUAL_INT(-50, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(20, snapshot.maximumRecorded);

  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = false;
  TEST_ASSERT_FALSE(tracker.record(observation));
  snapshot = tracker.snapshot();
  TEST_ASSERT_FALSE(snapshot.hasPressure);
  TEST_ASSERT_EQUAL_INT(-50, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(20, snapshot.maximumRecorded);
}

/// Only fresh valid observations fill a trend slot; stale intervals remain gaps.
void test_measurement_tracker_marks_missing_samples_as_gaps() {
  MeasurementTracker tracker;
  TEST_ASSERT_EQUAL_UINT(0, tracker.sampleAt(0));

  MeasurementObservation observation;
  observation.source = MeasurementSource::kLive;
  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = -60;
  observation.acquiredAtMs = 400;
  TEST_ASSERT_TRUE(tracker.record(observation));
  TEST_ASSERT_EQUAL_UINT(1, tracker.sampleAt(1000));

  observation.quality = MeasurementQuality::kStale;
  observation.hasPressure = false;
  observation.acquiredAtMs = 1500;
  TEST_ASSERT_TRUE(tracker.record(observation));
  TEST_ASSERT_EQUAL_UINT(1, tracker.sampleAt(2000));

  const MeasurementSnapshot snapshot = tracker.snapshot();
  TEST_ASSERT_EQUAL_UINT(2, snapshot.historyCount);
  TEST_ASSERT_TRUE(snapshot.history[0].valid);
  TEST_ASSERT_EQUAL_INT(-60, snapshot.history[0].pressure);
  TEST_ASSERT_FALSE(snapshot.history[1].valid);
  TEST_ASSERT_EQUAL_UINT(2000, snapshot.history[1].timestampMs);
}

/// A valid observation just after a due boundary belongs to the latest due slot.
void test_measurement_tracker_handles_sampling_lateness_and_millis_rollover() {
  MeasurementTracker tracker;
  TEST_ASSERT_EQUAL_UINT(0, tracker.sampleAt(100));
  MeasurementObservation observation;
  observation.source = MeasurementSource::kLive;
  observation.quality = MeasurementQuality::kValid;
  observation.hasPressure = true;
  observation.pressure = -40;
  observation.acquiredAtMs = 1105;
  TEST_ASSERT_TRUE(tracker.record(observation));
  TEST_ASSERT_EQUAL_UINT(1, tracker.sampleAt(1105));
  TEST_ASSERT_TRUE(tracker.snapshot().history[0].valid);

  MeasurementTracker rolloverTracker;
  TEST_ASSERT_EQUAL_UINT(0, rolloverTracker.sampleAt(0xFFFFFF00u));
  observation.acquiredAtMs = 0x000002E8u;
  TEST_ASSERT_TRUE(rolloverTracker.record(observation));
  TEST_ASSERT_EQUAL_UINT(1, rolloverTracker.sampleAt(0x000002E8u));
  TEST_ASSERT_TRUE(rolloverTracker.snapshot().history[0].valid);
}

/// Accept a bounded stable capture and report its rounded mean and observed range.
void test_zero_capture_accepts_stable_samples() {
  const std::uint32_t samples[] = {1500000, 1500100, 1499900};
  ZeroCaptureCriteria criteria;
  criteria.minimumVoltageMicrovolts = 500000;
  criteria.maximumVoltageMicrovolts = 4500000;
  criteria.maximumPeakToPeakMicrovolts = 200;
  criteria.minimumSampleCount = 3;
  const ZeroCaptureResult result =
      ZeroReferenceCapture().evaluate(samples, 3, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kAccepted),
                    static_cast<int>(result.status));
  TEST_ASSERT_EQUAL_UINT(1500000, result.zeroVoltageMicrovolts);
  TEST_ASSERT_EQUAL_UINT(1499900, result.minimumObservedMicrovolts);
  TEST_ASSERT_EQUAL_UINT(1500100, result.maximumObservedMicrovolts);
}

/// Reject incomplete, implausible, unstable, and unbounded zero-capture windows.
void test_zero_capture_rejects_invalid_windows() {
  const std::uint32_t stable[] = {1500000, 1500100, 1499900};
  const std::uint32_t unstable[] = {1500000, 1501000, 1499000};
  ZeroCaptureCriteria criteria;
  criteria.minimumVoltageMicrovolts = 500000;
  criteria.maximumVoltageMicrovolts = 4500000;
  criteria.maximumPeakToPeakMicrovolts = 200;
  criteria.minimumSampleCount = 3;
  const ZeroReferenceCapture capture;

  ZeroCaptureResult result = capture.evaluate(stable, 2, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kInsufficientSamples),
                    static_cast<int>(result.status));
  result = capture.evaluate(unstable, 3, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kUnstable),
                    static_cast<int>(result.status));

  const std::uint32_t outside[] = {1500000, 4500001, 1500000};
  result = capture.evaluate(outside, 3, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kVoltageOutOfRange),
                    static_cast<int>(result.status));
  result = capture.evaluate(nullptr, 3, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kInsufficientSamples),
                    static_cast<int>(result.status));

  criteria.minimumSampleCount = 1;
  result = capture.evaluate(stable, 3, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kInvalidCriteria),
                    static_cast<int>(result.status));

  std::uint32_t tooMany[kMaximumZeroCaptureSamples + 1] = {};
  criteria.minimumSampleCount = 2;
  result = capture.evaluate(tooMany, kMaximumZeroCaptureSamples + 1, criteria);
  TEST_ASSERT_EQUAL(static_cast<int>(ZeroCaptureStatus::kTooManySamples),
                    static_cast<int>(result.status));
}

/// Unity's native runner invokes the same named tests on the development host.
int main() {
  UNITY_BEGIN();
  RUN_TEST(test_default_demo_configuration_and_span);
  RUN_TEST(test_classifier_threshold_edges);
  RUN_TEST(test_classifier_separates_status_limits_from_measurement_domain);
  RUN_TEST(test_demo_values_are_bounded_and_classified_for_requested_band);
  RUN_TEST(test_demo_source_is_repeatable_for_equal_seeds);
  RUN_TEST(test_normal_generator_rejects_invalid_configuration);
  RUN_TEST(test_sampling_starts_after_first_observation_and_uses_elapsed_seconds);
  RUN_TEST(test_sampling_handles_millis_rollover);
  RUN_TEST(test_history_is_partial_then_rolls_over_oldest_samples);
  RUN_TEST(test_tracker_catches_up_with_gaps_and_caps_work_to_ring_size);
  RUN_TEST(test_demo_history_preserves_repeated_state_during_catch_up);
  RUN_TEST(test_extrema_track_accepted_values_and_reject_out_of_span);
  RUN_TEST(test_gestures_change_demo_state_and_pages_without_sampling);
  RUN_TEST(test_pressure_converter_family_anchors_and_sign);
  RUN_TEST(test_pressure_converter_rounding_and_invalid_inputs);
  RUN_TEST(test_measurement_tracker_invalid_states_preserve_valid_extrema);
  RUN_TEST(test_measurement_tracker_marks_missing_samples_as_gaps);
  RUN_TEST(test_measurement_tracker_handles_sampling_lateness_and_millis_rollover);
  RUN_TEST(test_zero_capture_accepts_stable_samples);
  RUN_TEST(test_zero_capture_rejects_invalid_windows);
  return UNITY_END();
}
