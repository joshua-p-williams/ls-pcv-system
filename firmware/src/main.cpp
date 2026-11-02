#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "board/n16r8/board_config.hpp"
#include "display/lvgl_gc9a01.hpp"
#include "gauge_core/demo_pressure_source.hpp"
#include "gauge_core/gauge_session.hpp"
#include "gauge_core/m01_interaction.hpp"
#include "input/better_button_input.hpp"

namespace {

// BetterButton is a polling state machine, so it must be called frequently.
// LVGL's synchronous SPI refresh can occupy Arduino's loop task for hundreds
// of milliseconds; a separate FreeRTOS task keeps button sampling independent.
// One FreeRTOS tick is short relative to BetterButton's 60 ms debounce window.
constexpr TickType_t kButtonPollIntervalTicks = 1;
// The queue transfers small gesture events only. UI and session objects remain
// owned by loop(), avoiding concurrent access to LVGL or gauge state.
constexpr std::size_t kPendingGestureCapacity = 8;

// These objects form the application's composition root: the synthetic source
// feeds the hardware-independent session through the temporary M01 interaction,
// while board-specific adapters connect the session to GPIO and the display.
gauge_core::DemoPressureSource demoPressureSource;
gauge_core::GaugeSession gaugeSession;
gauge_core::M01Interaction interaction(demoPressureSource, gaugeSession);
ls_pcv::input::BetterButtonInput buttonInput(ls_pcv::board::n16r8::kButtonPin);
ls_pcv::display::LvglGc9a01 gaugeDisplay;
bool displayReady = false;
QueueHandle_t gestureQueue = nullptr;
bool buttonPollingTaskReady = false;

/// Return a lowercase status name for USB diagnostic records.
const char* pressureBandName(gauge_core::PressureBand band) {
  switch (band) {
    case gauge_core::PressureBand::kDanger: return "danger";
    case gauge_core::PressureBand::kWarning: return "warning";
    case gauge_core::PressureBand::kNormal: return "normal";
  }
  return "unknown";
}

/// Return a concise stable page name for optional USB diagnostics.
const char* pageName(gauge_core::Page page) {
  switch (page) {
    case gauge_core::Page::kGauge: return "gauge";
    case gauge_core::Page::kTrend: return "trend";
    case gauge_core::Page::kMinMax: return "min-max";
  }
  return "unknown";
}

/// Return a concise stable gesture name for optional USB diagnostics.
const char* gestureName(gauge_core::Gesture gesture) {
  switch (gesture) {
    case gauge_core::Gesture::kClick: return "click";
    case gauge_core::Gesture::kDoubleClick: return "double-click";
    case gauge_core::Gesture::kLongPress: return "long-press";
  }
  return "unknown";
}

/// Print the current synthetic session state without making serial mandatory.
/// Pressure is logged in tenths of inH2O to avoid floating-point formatting.
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
      static_cast<int>(snapshot.minimumRecorded), static_cast<int>(snapshot.maximumRecorded));
}

/// Poll the button independently of display rendering and queue recognized input.
/// A zero-timeout queue send keeps this higher-priority task nonblocking; if the
/// queue is full, dropping the newest gesture is preferable to stalling polling.
void buttonPollingTask(void*) {
  for (;;) {
    gauge_core::Gesture gesture;
    if (buttonInput.poll(gesture)) {
      // The main loop is servicing the display and the bounded gesture queue
      // is full. Drop a new event rather than blocking this input task.
      (void)xQueueSend(gestureQueue, &gesture, 0);
    }
    vTaskDelay(kButtonPollIntervalTicks);
  }
}

/// Apply a gesture and refresh the UI from a fresh immutable session snapshot.
/// This function runs only on the Arduino loop task, which owns LVGL access.
void processGesture(gauge_core::Gesture gesture) {
  if (interaction.handle(gesture)) {
    reportDemoState(gestureName(gesture));
    gaugeDisplay.render(gaugeSession.snapshot());
  } else {
    Serial.printf("M01 error=gesture action failed event=%s\n", gestureName(gesture));
  }
}

}  // namespace

/// Arduino framework entry point called once after reset.
/// Initializes diagnostics, input, display and the gesture queue, then seeds the
/// display with one synthetic normal value. Startup never waits for USB serial.
void setup() {
  Serial.begin(115200);
  // USB can remain physically connected after its monitor stops reading. Keep
  // diagnostics best-effort so a full CDC transmit queue cannot stall button
  // polling, LVGL servicing, or the one-second history sampler.
  Serial.setTxTimeoutMs(0);
  Serial.println("LS PCV gauge firmware started");
  Serial.println("M01 input=BetterButton GPIO6; source=synthetic; display=not initialized");

  // BetterButton configures GPIO6 as INPUT_PULLUP. There is no device-detection
  // protocol for a passive switch; an absent or open switch simply emits no event.
  buttonInput.begin();

  displayReady = gaugeDisplay.begin(gaugeSession.snapshot().bands);
  Serial.printf("M01 display=%s driver=GC9A01 stack=LVGL-9.2.2/LovyanGFX-1.2.35\n",
                displayReady ? "ready" : "unavailable");

  gestureQueue = xQueueCreate(kPendingGestureCapacity, sizeof(gauge_core::Gesture));
  if (gestureQueue != nullptr) {
    // ESP32 runs loop() in a low-priority Arduino task. Priority 2 gives the
    // short GPIO poll task a chance to observe brief presses during SPI flushes.
    // It sends only enum values; it never calls LVGL or edits GaugeSession.
    buttonPollingTaskReady =
        xTaskCreateUniversal(buttonPollingTask, "buttonPoll", 3072, nullptr, 2,
                             nullptr, ARDUINO_RUNNING_CORE) == pdPASS;
  }
  if (!buttonPollingTaskReady) {
    Serial.println("M01 error=button polling task unavailable; using loop fallback");
  }

  // Begin with a useful synthetic normal value so long-press/page behavior and
  // USB diagnostics have session state before the first physical button event.
  const gauge_core::DemoBandConfig bands = gaugeSession.snapshot().bands;
  gauge_core::PressureTenthsInH2O initialPressure = 0;
  if (demoPressureSource.nextNormal(bands, initialPressure) &&
      gaugeSession.updatePressure(initialPressure)) {
    reportDemoState("startup");
    if (displayReady) gaugeDisplay.render(gaugeSession.snapshot());
  } else {
    Serial.println("M01 error=unable to initialize synthetic pressure state");
  }
}

/// Arduino framework's repeatedly called application task.
/// Drain input events, collect one-second trend samples, and service LVGL timers.
/// There are no long delays: display animations and history timing depend on
/// this function being called often, while the separate task polls the button.
void loop() {
  const std::uint32_t now = millis();
  gauge_core::Gesture gesture;
  if (buttonPollingTaskReady) {
    while (xQueueReceive(gestureQueue, &gesture, 0) == pdTRUE) {
      processGesture(gesture);
    }
  } else if (buttonInput.poll(gesture)) {
    processGesture(gesture);
  }

  // Keep trend collection independent of the selected page and button events.
  const std::size_t samplesAdded = gaugeSession.sampleAt(now);
  if (displayReady) {
    if (samplesAdded > 0) gaugeDisplay.render(gaugeSession.snapshot());
    gaugeDisplay.service();
  }
}
