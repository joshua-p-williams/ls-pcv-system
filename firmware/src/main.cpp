#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "board/n16r8/board_config.hpp"
#include "display/lvgl_gc9a01.hpp"
#include "input/better_button_input.hpp"

#if defined(LS_PCV_DEMO_BUILD)
#include "gauge_core/demo_pressure_source.hpp"
#include "gauge_core/gauge_session.hpp"
#include "gauge_core/m01_interaction.hpp"
#else
#include <Wire.h>
#include "board/n16r8/arduino_wire_bus.hpp"
#include "gauge_core/gauge_types.hpp"
#include "gauge_core/measurement_freshness.hpp"
#include "measurement/ads1115_pressure_source.hpp"
#include "measurement/preferences_zero_store.hpp"
#endif

namespace {

// BetterButton is a polling state machine. A separate short-interval task keeps
// it observing the GPIO even when the synchronous SPI display refresh occupies
// the Arduino loop long enough that a quick button press could otherwise pass.
constexpr TickType_t kButtonPollIntervalTicks = 1;
constexpr std::size_t kPendingGestureCapacity = 8;
QueueHandle_t gestureQueue = nullptr;
bool buttonPollingTaskReady = false;
bool displayReady = false;
ls_pcv::input::BetterButtonInput buttonInput(
    ls_pcv::board::n16r8::kButtonPin);
ls_pcv::display::LvglGc9a01 gaugeDisplay;

#if defined(LS_PCV_DEMO_BUILD)

// The demo composition intentionally has no Wire, ADS1115, or sensor objects.
// Its synthetic source and M01Interaction preserve the original demonstration
// without being able to report a synthetic value as a physical measurement.
gauge_core::DemoPressureSource demoPressureSource;
gauge_core::GaugeSession gaugeSession;
gauge_core::M01Interaction interaction(demoPressureSource, gaugeSession);

/// Return a lowercase status name for optional USB demo diagnostics.
const char* pressureBandName(gauge_core::PressureBand band) {
  switch (band) {
    case gauge_core::PressureBand::kDanger: return "danger";
    case gauge_core::PressureBand::kWarning: return "warning";
    case gauge_core::PressureBand::kNormal: return "normal";
  }
  return "unknown";
}

const char* pageName(gauge_core::Page page) {
  switch (page) {
    case gauge_core::Page::kGauge: return "gauge";
    case gauge_core::Page::kTrend: return "trend";
    case gauge_core::Page::kMinMax: return "min-max";
    case gauge_core::Page::kSettings: return "settings";
  }
  return "unknown";
}

const char* gestureName(gauge_core::Gesture gesture) {
  switch (gesture) {
    case gauge_core::Gesture::kClick: return "click";
    case gauge_core::Gesture::kDoubleClick: return "double-click";
    case gauge_core::Gesture::kLongPress: return "long-press";
  }
  return "unknown";
}

/// Print the current synthetic session state; pressure uses tenths of inH2O.
void reportDemoState(const char* eventName) {
  const gauge_core::GaugeDisplaySnapshot snapshot = gaugeSession.snapshot();
  if (!snapshot.hasPressure) {
    Serial.printf("M01 source=synthetic event=%s pressure=unavailable page=%s\n",
                  eventName, pageName(snapshot.page));
    return;
  }
  Serial.printf(
      "M01 source=synthetic event=%s pressure=%d tenths_inH2O status=%s page=%s "
      "history=%u/%u min=%d max=%d tenths_inH2O\n",
      eventName, static_cast<int>(snapshot.pressure), pressureBandName(snapshot.band),
      pageName(snapshot.page), static_cast<unsigned>(snapshot.historyCount),
      static_cast<unsigned>(gauge_core::kHistoryCapacity),
      static_cast<int>(snapshot.minimumRecorded),
      static_cast<int>(snapshot.maximumRecorded));
}

/// Apply a temporary M01 demo gesture in the application/loop context.
void processGesture(gauge_core::Gesture gesture) {
  if (interaction.handle(gesture)) {
    reportDemoState(gestureName(gesture));
    gaugeDisplay.render(gaugeSession.snapshot());
  } else {
    Serial.printf("M01 error=gesture action failed event=%s\n", gestureName(gesture));
  }
}

#else

// In live builds, only this acquisition task owns Wire and the mutable pressure
// tracker. A validated Preferences record may replace the documented 1.500 V
// family reference only after an explicit atmospheric-zero confirmation.
constexpr std::uint32_t kAcquisitionTaskDelayMs = 2;
constexpr std::uint32_t kAcquisitionTaskStackBytes = 4096;
constexpr UBaseType_t kAcquisitionTaskPriority = 2;
constexpr std::uint32_t kDisplayRefreshIntervalMs = 100;
constexpr std::uint32_t kDiagnosticIntervalMs = 250;
// A one-second trend period plus half a second of scheduling margin. This is a
// freshness policy, not a sensor specification or engine pressure threshold.
constexpr std::uint32_t kStaleAfterMs = 1500;
constexpr std::uint32_t kSettingsConfirmationTimeoutMs = 30000;
constexpr std::uint32_t kSettingsSuccessDisplayMs = 2500;

/// Commands cross into the sensor task; the application never touches I2C/NVS.
enum class MeasurementCommand : std::uint8_t {
  kBeginZeroCapture,
  kCancelZeroCapture
};

ls_pcv::board::n16r8::ArduinoWireBus i2cBus(
    Wire, ls_pcv::board::n16r8::kI2cSdaPin,
    ls_pcv::board::n16r8::kI2cSclPin,
    ls_pcv::board::n16r8::kI2cFrequencyHz,
    ls_pcv::board::n16r8::kI2cTransactionTimeoutMs);
ls_pcv::measurement::PreferencesZeroStore zeroStore;
ls_pcv::measurement::Ads1115PressureSource pressureSource(
    i2cBus, ls_pcv::board::n16r8::kAds1115Address, {}, {}, &zeroStore);
QueueHandle_t measurementMailbox = nullptr;
QueueHandle_t measurementCommandQueue = nullptr;
gauge_core::MeasurementSnapshot latestMeasurement{};
gauge_core::DemoBandConfig displayBands{};
gauge_core::Page activePage = gauge_core::Page::kGauge;
gauge_core::SettingsView settingsView = gauge_core::SettingsView::kMenu;
bool settingsConfirmSelected = false;
bool settingsCommandFailed = false;
std::uint32_t lastSettingsInteractionMs = 0;
std::uint32_t settingsResultStartedAtMs = 0;
std::uint32_t settingsCaptureRequestedAtMs = 0;
bool acquisitionTaskReady = false;
bool hasMeasurement = false;

const char* qualityName(gauge_core::MeasurementQuality quality) {
  switch (quality) {
    case gauge_core::MeasurementQuality::kUnavailable: return "unavailable";
    case gauge_core::MeasurementQuality::kValid: return "valid";
    case gauge_core::MeasurementQuality::kStale: return "stale";
    case gauge_core::MeasurementQuality::kAdcError: return "adc-error";
    case gauge_core::MeasurementQuality::kI2cError: return "i2c-error";
    case gauge_core::MeasurementQuality::kOutOfDomain: return "out-of-domain";
  }
  return "unknown";
}

const char* bandName(gauge_core::PressureBand band) {
  switch (band) {
    case gauge_core::PressureBand::kDanger: return "danger";
    case gauge_core::PressureBand::kWarning: return "warning";
    case gauge_core::PressureBand::kNormal: return "normal";
  }
  return "unknown";
}

const char* pageName(gauge_core::Page page) {
  switch (page) {
    case gauge_core::Page::kGauge: return "gauge";
    case gauge_core::Page::kTrend: return "trend";
    case gauge_core::Page::kMinMax: return "min-max";
    case gauge_core::Page::kSettings: return "settings";
  }
  return "unknown";
}

/// Describe the active/reference lifecycle in optional live diagnostics.
const char* zeroReferenceName(gauge_core::ZeroReferenceState state) {
  switch (state) {
    case gauge_core::ZeroReferenceState::kFamilyDefault: return "family-default";
    case gauge_core::ZeroReferenceState::kSavedTrim: return "saved";
    case gauge_core::ZeroReferenceState::kCaptureInProgress: return "capturing";
    case gauge_core::ZeroReferenceState::kSavedSuccessfully: return "saved-now";
    case gauge_core::ZeroReferenceState::kRejected: return "rejected";
    case gauge_core::ZeroReferenceState::kStorageFailure: return "storage-error";
    case gauge_core::ZeroReferenceState::kInvalidStoredData: return "invalid-record";
  }
  return "unknown";
}

/// Add live measurement and application-owned page state for LVGL. Marking a
/// value stale affects only this presentation copy, never the tracker's history.
gauge_core::GaugeDisplaySnapshot displaySnapshot(std::uint32_t nowMs) {
  gauge_core::GaugeDisplaySnapshot view;
  static_cast<gauge_core::MeasurementSnapshot&>(view) =
      gauge_core::withFreshnessPolicy(latestMeasurement, nowMs, kStaleAfterMs);
  view.bands = displayBands;
  view.page = activePage;
  view.settingsView = settingsView;
  view.settingsConfirmSelected = settingsConfirmSelected;
  view.settingsCommandFailed = settingsCommandFailed;
  return view;
}

/// Poll the ADS1115 state machine and publish each completed result/fault.
/// History/extrema are processed before the one-slot mailbox replaces an older
/// display frame. This task never calls LVGL or writes optional USB diagnostics.
void acquisitionTask(void*) {
  for (;;) {
    MeasurementCommand command;
    bool changed = false;
    while (measurementCommandQueue != nullptr &&
           xQueueReceive(measurementCommandQueue, &command, 0) == pdTRUE) {
      if (command == MeasurementCommand::kBeginZeroCapture) {
        (void)pressureSource.beginZeroCapture();
      } else {
        pressureSource.cancelZeroCapture();
      }
      changed = true;
    }
    if (pressureSource.poll(millis())) changed = true;
    if (changed) {
      const gauge_core::MeasurementSnapshot snapshot = pressureSource.snapshot();
      if (measurementMailbox != nullptr) {
        (void)xQueueOverwrite(measurementMailbox, &snapshot);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(kAcquisitionTaskDelayMs));
  }
}

/// Print the latest sensor state at a bounded rate, without gating the gauge.
void reportSnapshot(const gauge_core::GaugeDisplaySnapshot& snapshot) {
  Serial.printf("M02_LIVE source=ftp-adc quality=%s",
                qualityName(snapshot.quality));
  if (snapshot.hasRawAdcCode) {
    Serial.printf(" raw_code=%d", static_cast<int>(snapshot.rawAdcCode));
  }
  if (snapshot.hasSignalVoltage) {
    Serial.printf(" a0_uV=%lu",
                  static_cast<unsigned long>(snapshot.signalVoltageMicrovolts));
  }
  if (snapshot.hasPressure &&
      snapshot.quality == gauge_core::MeasurementQuality::kValid) {
    Serial.printf(" pressure=%d tenths_inH2O band=%s",
                  static_cast<int>(snapshot.pressure), bandName(snapshot.band));
  } else {
    Serial.print(" pressure=unavailable");
  }
  Serial.printf(" page=%s history=%u/%u zero=%s\n",
                pageName(snapshot.page),
                static_cast<unsigned>(snapshot.historyCount),
                static_cast<unsigned>(gauge_core::kHistoryCapacity),
                zeroReferenceName(snapshot.zeroReference));
}

/// Deliver a zero command to its single owner without waiting for that task.
bool sendMeasurementCommand(MeasurementCommand command) {
  return measurementCommandQueue != nullptr &&
      xQueueSend(measurementCommandQueue, &command, 0) == pdTRUE;
}

/// Route long press among live pages; clicks are meaningful only in Settings.
void processGesture(gauge_core::Gesture gesture, std::uint32_t nowMs) {
  if (gesture == gauge_core::Gesture::kLongPress) {
    settingsCommandFailed = false;
    if (activePage == gauge_core::Page::kSettings) {
      if (settingsView == gauge_core::SettingsView::kConfirm) {
        // Long press is a safe escape from the confirmation prompt.
        settingsView = gauge_core::SettingsView::kMenu;
        settingsConfirmSelected = false;
      } else if (settingsView == gauge_core::SettingsView::kCapturing) {
        // The capture has not written anything until the final accepted sample.
        settingsCommandFailed = !sendMeasurementCommand(
            MeasurementCommand::kCancelZeroCapture);
        settingsView = gauge_core::SettingsView::kResult;
        settingsResultStartedAtMs = nowMs;
      } else if (settingsView == gauge_core::SettingsView::kResult) {
        activePage = gauge_core::Page::kGauge;
        settingsView = gauge_core::SettingsView::kMenu;
      } else {
        activePage = gauge_core::Page::kGauge;
      }
      return;
    }
    switch (activePage) {
      case gauge_core::Page::kGauge: activePage = gauge_core::Page::kTrend; break;
      case gauge_core::Page::kTrend: activePage = gauge_core::Page::kMinMax; break;
      case gauge_core::Page::kMinMax: activePage = gauge_core::Page::kSettings; break;
      case gauge_core::Page::kSettings: break;
    }
    settingsView = gauge_core::SettingsView::kMenu;
    settingsConfirmSelected = false;
    lastSettingsInteractionMs = nowMs;
    return;
  }

  if (activePage != gauge_core::Page::kSettings) return;
  lastSettingsInteractionMs = nowMs;
  switch (settingsView) {
    case gauge_core::SettingsView::kMenu:
      // There is one action today. A single click cycles the one-item menu;
      // the established gesture remains ready for additional settings later.
      if (gesture == gauge_core::Gesture::kDoubleClick) {
        settingsView = gauge_core::SettingsView::kConfirm;
        settingsConfirmSelected = false;  // Cancel is always the first choice.
        settingsCommandFailed = false;
      }
      break;
    case gauge_core::SettingsView::kConfirm:
      if (gesture == gauge_core::Gesture::kClick) {
        settingsConfirmSelected = !settingsConfirmSelected;
      } else if (gesture == gauge_core::Gesture::kDoubleClick) {
        if (!settingsConfirmSelected) {
          settingsView = gauge_core::SettingsView::kMenu;
        } else if (sendMeasurementCommand(
                       MeasurementCommand::kBeginZeroCapture)) {
          settingsView = gauge_core::SettingsView::kCapturing;
          settingsCaptureRequestedAtMs = nowMs;
        } else {
          settingsCommandFailed = true;
          settingsView = gauge_core::SettingsView::kResult;
          settingsResultStartedAtMs = nowMs;
        }
      }
      break;
    case gauge_core::SettingsView::kCapturing:
      // The capture is a short, finite acquisition window; gestures cannot
      // alter its sample set or interrupt a write already in progress.
      break;
    case gauge_core::SettingsView::kResult:
      if (gesture == gauge_core::Gesture::kClick ||
          gesture == gauge_core::Gesture::kDoubleClick) {
        settingsView = gauge_core::SettingsView::kMenu;
        settingsCommandFailed = false;
      }
      break;
  }
}

/// Cancel unattended confirmation and return after a visible successful result.
void updateSettingsTimers(std::uint32_t nowMs) {
  if (activePage != gauge_core::Page::kSettings) return;
  if (settingsView == gauge_core::SettingsView::kConfirm &&
      static_cast<std::uint32_t>(nowMs - lastSettingsInteractionMs) >=
          kSettingsConfirmationTimeoutMs) {
    settingsView = gauge_core::SettingsView::kMenu;
    settingsConfirmSelected = false;
  }

  if (settingsView == gauge_core::SettingsView::kCapturing && hasMeasurement) {
    const bool snapshotIsNewerThanRequest =
        static_cast<std::uint32_t>(latestMeasurement.acquiredAtMs -
                                   settingsCaptureRequestedAtMs) < 0x80000000u;
    if (snapshotIsNewerThanRequest &&
        latestMeasurement.zeroReference !=
            gauge_core::ZeroReferenceState::kCaptureInProgress) {
      settingsView = gauge_core::SettingsView::kResult;
      settingsResultStartedAtMs = nowMs;
    }
  }

  if (settingsView == gauge_core::SettingsView::kResult && hasMeasurement &&
      latestMeasurement.zeroReference ==
          gauge_core::ZeroReferenceState::kSavedSuccessfully &&
      static_cast<std::uint32_t>(nowMs - settingsResultStartedAtMs) >=
          kSettingsSuccessDisplayMs) {
    activePage = gauge_core::Page::kGauge;
    settingsView = gauge_core::SettingsView::kMenu;
  }
}

#endif

/// Adapt the common gesture queue to the demo or live application signature.
void handleGesture(gauge_core::Gesture gesture) {
#if defined(LS_PCV_DEMO_BUILD)
  processGesture(gesture);
#else
  processGesture(gesture, millis());
#endif
}

/// The polling task sends only gesture enums. LVGL and application state remain
/// owned by loop(), and a full queue never stalls input sampling.
void buttonPollingTask(void*) {
  for (;;) {
    gauge_core::Gesture gesture;
    if (buttonInput.poll(gesture) && gestureQueue != nullptr) {
      (void)xQueueSend(gestureQueue, &gesture, 0);
    }
    vTaskDelay(kButtonPollIntervalTicks);
  }
}

}  // namespace

/// Initialize the chosen build-time composition. USB diagnostics are optional
/// and transmit immediately rather than waiting for a host to open the monitor.
void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  buttonInput.begin();
  gestureQueue = xQueueCreate(kPendingGestureCapacity, sizeof(gauge_core::Gesture));
  displayReady = gaugeDisplay.begin(
#if defined(LS_PCV_DEMO_BUILD)
      gaugeSession.snapshot().bands, false
#else
      displayBands, true
#endif
  );

#if defined(LS_PCV_DEMO_BUILD)
  Serial.println("LS PCV M01 synthetic display demonstration");
  Serial.println("source=synthetic; input=BetterButton; sensor/ADC=not initialized");
#else
  Serial.println("LS PCV M02 live sensor-to-display build");
  Serial.println("source=FTP/ADS1115; default zero=1.500 V family reference");
  measurementMailbox = xQueueCreate(1, sizeof(gauge_core::MeasurementSnapshot));
  measurementCommandQueue = xQueueCreate(2, sizeof(MeasurementCommand));
  if (measurementMailbox != nullptr && measurementCommandQueue != nullptr) {
    acquisitionTaskReady = xTaskCreateUniversal(
        acquisitionTask, "ads1115Acquire", kAcquisitionTaskStackBytes,
        nullptr, kAcquisitionTaskPriority, nullptr,
        ARDUINO_RUNNING_CORE) == pdPASS;
  }
#endif

  if (gestureQueue != nullptr) {
    buttonPollingTaskReady = xTaskCreateUniversal(
        buttonPollingTask, "buttonPoll", 3072, nullptr, 2,
        nullptr, ARDUINO_RUNNING_CORE) == pdPASS;
  }

#if defined(LS_PCV_DEMO_BUILD)
  Serial.printf("M01 display=%s driver=GC9A01 stack=LVGL-9.2.2/LovyanGFX-1.2.35\n",
                displayReady ? "ready" : "unavailable");
  gauge_core::PressureTenthsInH2O initialPressure = 0;
  const gauge_core::DemoBandConfig bands = gaugeSession.snapshot().bands;
  if (demoPressureSource.nextNormal(bands, initialPressure) &&
      gaugeSession.updatePressure(initialPressure)) {
    reportDemoState("startup");
    if (displayReady) gaugeDisplay.render(gaugeSession.snapshot());
  } else {
    Serial.println("M01 error=unable to initialize synthetic pressure state");
  }
#else
  Serial.printf("M02 startup display=%s acquisition=%s button_poll=%s "
                "adc=0x%02X sda=%u scl=%u\n",
                displayReady ? "ready" : "unavailable",
                acquisitionTaskReady ? "ready" : "unavailable",
                buttonPollingTaskReady ? "ready" : "loop-fallback",
                static_cast<unsigned>(ls_pcv::board::n16r8::kAds1115Address),
                static_cast<unsigned>(ls_pcv::board::n16r8::kI2cSdaPin),
                static_cast<unsigned>(ls_pcv::board::n16r8::kI2cSclPin));
#endif
}

/// Arduino framework calls loop() repeatedly. The demo records its synthetic
/// history here; live acquisition updates its tracker in another task so neither
/// mode makes history depend on display rendering or monitor availability.
void loop() {
  gauge_core::Gesture gesture;
  if (buttonPollingTaskReady) {
    while (gestureQueue != nullptr &&
           xQueueReceive(gestureQueue, &gesture, 0) == pdTRUE) {
      handleGesture(gesture);
    }
  } else if (buttonInput.poll(gesture)) {
    handleGesture(gesture);
  }

#if defined(LS_PCV_DEMO_BUILD)
  const std::size_t samplesAdded = gaugeSession.sampleAt(millis());
  if (displayReady) {
    if (samplesAdded > 0) gaugeDisplay.render(gaugeSession.snapshot());
    gaugeDisplay.service();
  }
#else
  static bool haveRendered = false;
  static std::uint32_t lastRenderMs = 0;
  static std::uint32_t lastDiagnosticMs = 0;
  static gauge_core::Page lastRenderedPage = gauge_core::Page::kGauge;
  static gauge_core::MeasurementQuality lastRenderedQuality =
      gauge_core::MeasurementQuality::kUnavailable;

  if (measurementMailbox != nullptr) {
    gauge_core::MeasurementSnapshot newest;
    if (xQueueReceive(measurementMailbox, &newest, 0) == pdTRUE) {
      latestMeasurement = newest;
      hasMeasurement = true;
    }
  }
  const std::uint32_t nowMs = millis();
  updateSettingsTimers(nowMs);
  if (hasMeasurement && displayReady) {
    const gauge_core::GaugeDisplaySnapshot view = displaySnapshot(nowMs);
    const bool pageChanged = view.page != lastRenderedPage;
    const bool qualityChanged = view.quality != lastRenderedQuality;
    if (!haveRendered || pageChanged || qualityChanged ||
        static_cast<std::uint32_t>(nowMs - lastRenderMs) >=
            kDisplayRefreshIntervalMs) {
      gaugeDisplay.render(view);
      lastRenderMs = nowMs;
      lastRenderedPage = view.page;
      lastRenderedQuality = view.quality;
      haveRendered = true;
    }
    if (static_cast<std::uint32_t>(nowMs - lastDiagnosticMs) >=
        kDiagnosticIntervalMs) {
      reportSnapshot(view);
      lastDiagnosticMs = nowMs;
    }
  } else if (displayReady &&
             (!haveRendered || activePage != lastRenderedPage)) {
    gauge_core::GaugeDisplaySnapshot waiting;
    waiting.bands = displayBands;
    waiting.page = activePage;
    gaugeDisplay.render(waiting);
    lastRenderedPage = activePage;
    lastRenderedQuality = waiting.quality;
    haveRendered = true;
    lastRenderMs = nowMs;
  }
  if (displayReady) gaugeDisplay.service();
  vTaskDelay(pdMS_TO_TICKS(1));
#endif
}
