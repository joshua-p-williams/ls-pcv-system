#include <unity.h>

#include "gauge_core/demo_pressure_source.hpp"
#include "gauge_core/gauge_session.hpp"
#include "gauge_core/m01_interaction.hpp"
#include "gauge_core/pressure_classifier.hpp"

using namespace gauge_core;

void setUp() {}
void tearDown() {}

void test_default_demo_configuration_and_span() {
  const DemoBandConfig config;
  TEST_ASSERT_TRUE(config.isValid());
  TEST_ASSERT_TRUE(config.contains(-140));
  TEST_ASSERT_TRUE(config.contains(45));
  TEST_ASSERT_FALSE(config.contains(-141));
  TEST_ASSERT_FALSE(config.contains(46));
}

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

void test_classifier_rejects_out_of_span_and_invalid_configuration() {
  const PressureClassifier classifier;
  PressureBand result = PressureBand::kNormal;
  TEST_ASSERT_FALSE(classifier.classify(-141, DemoBandConfig{}, result));
  DemoBandConfig invalid;
  invalid.lowerWarning = -130;
  TEST_ASSERT_FALSE(classifier.classify(0, invalid, result));
}

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

void test_normal_generator_rejects_invalid_configuration() {
  DemoBandConfig invalid;
  invalid.upperDanger = -50;
  DemoPressureSource source;
  PressureTenthsInH2O value = 0;
  TEST_ASSERT_FALSE(source.nextNormal(invalid, value));
  TEST_ASSERT_FALSE(source.nextWarningOrDanger(invalid, value));
}

void test_sampling_starts_after_first_observation_and_uses_elapsed_seconds() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-55));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(100));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(1099));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(1100));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(2099));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(2100));
  DisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL_UINT(2, snapshot.historyCount);
  TEST_ASSERT_EQUAL_INT(-55, snapshot.history[0]);
  TEST_ASSERT_EQUAL_INT(-55, snapshot.history[1]);
}

void test_sampling_handles_millis_rollover() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-50));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0xFFFFFF00u));
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0x000002E7u));
  TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(0x000002E8u));
}

void test_history_is_partial_then_rolls_over_oldest_samples() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-70));
  TEST_ASSERT_EQUAL_UINT(0, session.snapshot().historyCount);
  TEST_ASSERT_EQUAL_UINT(0, session.sampleAt(0));
  for (int i = 1; i <= 35; ++i) {
    TEST_ASSERT_TRUE(session.updatePressure(static_cast<PressureTenthsInH2O>(-70 + i)));
    TEST_ASSERT_EQUAL_UINT(1, session.sampleAt(static_cast<std::uint32_t>(i) * 1000u));
  }
  const DisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_EQUAL_UINT(30, snapshot.historyCount);
  TEST_ASSERT_EQUAL_INT(-64, snapshot.history[0]);
  TEST_ASSERT_EQUAL_INT(-35, snapshot.history[29]);
}

void test_sampling_catches_up_for_elapsed_slots_but_caps_work_to_ring_size() {
  GaugeSession session;
  TEST_ASSERT_TRUE(session.updatePressure(-60));
  session.sampleAt(0);
  TEST_ASSERT_EQUAL_UINT(4, session.sampleAt(4000));
  TEST_ASSERT_EQUAL_UINT(30, session.sampleAt(100000));
  TEST_ASSERT_EQUAL_UINT(30, session.snapshot().historyCount);
}

void test_extrema_track_accepted_values_and_reject_out_of_span() {
  GaugeSession session;
  TEST_ASSERT_FALSE(session.snapshot().hasExtrema);
  TEST_ASSERT_TRUE(session.updatePressure(-50));
  TEST_ASSERT_TRUE(session.updatePressure(20));
  TEST_ASSERT_TRUE(session.updatePressure(-130));
  TEST_ASSERT_FALSE(session.updatePressure(46));
  const DisplaySnapshot snapshot = session.snapshot();
  TEST_ASSERT_TRUE(snapshot.hasExtrema);
  TEST_ASSERT_EQUAL_INT(-130, snapshot.minimumRecorded);
  TEST_ASSERT_EQUAL_INT(20, snapshot.maximumRecorded);
  TEST_ASSERT_EQUAL_INT(-130, snapshot.pressure);
}

void test_gestures_change_demo_state_and_pages_without_sampling() {
  DemoPressureSource source(456);
  GaugeSession session;
  M01Interaction interaction(source, session);

  TEST_ASSERT_TRUE(interaction.handle(Gesture::kClick));
  DisplaySnapshot snapshot = session.snapshot();
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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_default_demo_configuration_and_span);
  RUN_TEST(test_classifier_threshold_edges);
  RUN_TEST(test_classifier_rejects_out_of_span_and_invalid_configuration);
  RUN_TEST(test_demo_values_are_bounded_and_classified_for_requested_band);
  RUN_TEST(test_demo_source_is_repeatable_for_equal_seeds);
  RUN_TEST(test_normal_generator_rejects_invalid_configuration);
  RUN_TEST(test_sampling_starts_after_first_observation_and_uses_elapsed_seconds);
  RUN_TEST(test_sampling_handles_millis_rollover);
  RUN_TEST(test_history_is_partial_then_rolls_over_oldest_samples);
  RUN_TEST(test_sampling_catches_up_for_elapsed_slots_but_caps_work_to_ring_size);
  RUN_TEST(test_extrema_track_accepted_values_and_reject_out_of_span);
  RUN_TEST(test_gestures_change_demo_state_and_pages_without_sampling);
  return UNITY_END();
}
