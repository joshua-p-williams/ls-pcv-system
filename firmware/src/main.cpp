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

// One FreeRTOS tick is short relative to BetterButton's 60 ms debounce window.
constexpr TickType_t kButtonPollIntervalTicks = 1;
constexpr std::size_t kPendingGestureCapacity = 8;

gauge_core::DemoPressureSource demoPressureSource;
gauge_core::GaugeSession gaugeSession;
gauge_core::M01Interaction interaction(demoPressureSource, gaugeSession);
ls_pcv::input::BetterButtonInput buttonInput(ls_pcv::board::n16r8::kButtonPin);
ls_pcv::display::LvglGc9a01 gaugeDisplay;
bool displayReady = false;
QueueHandle_t gestureQueue = nullptr;
bool buttonPollingTaskReady = false;

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

void reportDemoState(const char* eventName) {
  const gauge_core::DisplaySnapshot snapshot = gaugeSession.snapshot();
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

void processGesture(gauge_core::Gesture gesture) {
  if (interaction.handle(gesture)) {
    reportDemoState(gestureName(gesture));
    gaugeDisplay.render(gaugeSession.snapshot());
  } else {
    Serial.printf("M01 error=gesture action failed event=%s\n", gestureName(gesture));
  }
}

}  // namespace

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
