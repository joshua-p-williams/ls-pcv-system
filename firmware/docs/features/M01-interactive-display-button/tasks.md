# Tasks: M01 — Interactive Display and Button

Tasks follow the reviewed [plan](plan.md). Keep items unchecked until the implementation or
verification has actually been completed; record observed results beside the relevant item. Host
tests establish application behavior, while the N16R8 checks establish behavior with the physical
button and display.

## Phase 1: Dependencies and input provenance

- [x] Inspect BetterButton source, identify the upstream repository revision to use, and confirm
  its API and license. Revision `e3a6bbaa6f72890d9dbb0105a754acc89c374ea3` is MIT-licensed and
  provides the planned GPIO-to-ground input-pull-up and click/double-click/long-press API. Its
  double-click window is measured from the initial press timestamp; verify practical timing during
  physical integration. See the [private library provenance](../../../lib/better-button/README.md).
- [x] Add the reviewed BetterButton source under the planned private firmware library. The
  upstream `.cpp`, `.h`, and MIT license are preserved; provenance and behavior notes are in the
  [library README](../../../lib/better-button/README.md). No second application implementation
  was added.
- [x] Add pinned LVGL 9.2.2 and LovyanGFX 1.2.35 dependencies to the existing N16R8 project
  configuration, following ADR 0001 and the known-good spike configuration. Reused the spike's
  LVGL configuration as the M01 starting configuration.
- [x] Confirm `n16r8` builds with the dependencies and BetterButton integration present. On
  Windows, PlatformIO resolved `espressif32@8.0.0`, Arduino-ESP32 3.3.12, LVGL 9.2.2, and
  LovyanGFX 1.2.35; it compiled `BetterButton.cpp` and completed successfully. The M00 heartbeat
  remains unchanged at runtime; the added includes are a compile check only. No hardware was
  initialized or uploaded during this phase.

## Phase 2: Gauge core and host tests

- [x] Create the hardware-independent `gauge-core` private library and define the signed pressure,
  band, gesture, page, demo configuration, and display snapshot types described by the architecture.
- [x] Implement the single shared demo-band configuration with a `-14` to `+4.5 inH2O` visual
  span and `-12`, `-8`, `-3`, and `0 inH2O` demo boundaries. Document in code that these values
  generate synthetic display states; they are not live sensor or engine thresholds.
- [x] Implement the signed pressure classifier with explicit inclusive/exclusive boundary
  behavior. Keep display ribbon sections derived from the same configuration as classification.
- [x] Implement reproducible normal-band and warning/danger synthetic value generation. Ensure
  generated values stay inside the visible span and allow tests to provide a deterministic seed or
  random source.
- [x] Implement the gauge session, current-run extrema, 30-entry history, one-second elapsed-time
  sampling, and temporary gesture/page transitions. Page changes must preserve the value, history,
  and extrema; button events must not insert history samples directly.
- [x] Configure a PlatformIO Unity host-test environment that compiles the core without
  `src/main.cpp`, Arduino, LVGL, LovyanGFX, or physical hardware dependencies.
- [x] Document the native GCC/toolchain prerequisites and exact setup/test commands for Windows and
  Linux in the firmware development guide. Keep OS-specific installation steps clearly identified.
- [x] Add focused host tests for classifier boundaries, generated-value band/range constraints and
  deterministic behavior, initial and rolling history, one-second timing, extrema, and
  gesture/page state transitions.
- [ ] Run the host test suite on Windows and native Linux using the same test sources. Record host
  OS, compiler/toolchain, PlatformIO version, command, and result for each platform; leave any
  unavailable platform explicitly pending. **Windows passed:** Windows 11 Pro, MSYS2 UCRT64 GCC
  and G++ 16.2.0, PlatformIO Core 6.2.0, native platform 1.2.1, Unity 2.6.1; command
  `platformio.exe test -e native` from the firmware directory in the UCRT64 shell (using the
  PlatformIO Core executable installed in the user profile); 12/12 tests passed. **Native Linux
  remains pending.**

## Phase 3: N16R8 button integration

- [x] Add the N16R8 button configuration using the W01 GPIO6 proposal and a GPIO pull-up; keep the
  board-specific pin definition out of the hardware-independent core.
- [x] Implement the BetterButton input adapter and translate click, double-click, and long-press
  events to the temporary M01 actions. The initial main-loop polling implementation was later
  replaced with a dedicated FreeRTOS polling task and bounded gesture queue after integrated timing
  measurements showed LVGL servicing could block the main loop for hundreds of milliseconds.
- [x] Keep the display/UI and gesture library out of the application-state rules. Ensure a missing
  or failed button does not stop the firmware loop or serial startup diagnostics.
- [x] Build and upload the N16R8 firmware. Manually verify click value generation, double-click
  warning/danger generation, long-press page transition, and USB diagnostics before adding display
  rendering. The N16R8 build passed with Espressif 32 8.0.0 / Arduino-ESP32 3.3.12 and compiled
  the adapter plus gauge core. On 2026-10-30, the uploaded board reported a click at `-61` tenths
  of `inH2O` as normal, a double-click at `+33` tenths as danger, and a long press that changed
  gauge → trend while preserving the pressure and extrema. Heartbeats continued between events.
  The log showed `history=30/30` when observed; because monitoring began after the board was running,
  this does not establish startup fill time or cadence. Values are synthetic; display integration
  and full physical page cycling remain for later verification.

## Phase 4: GC9A01 and three-page display

- [x] Add the N16R8 display configuration using W01's GPIO4 DC, GPIO5 reset, GPIO10 CS, GPIO11
  MOSI, and GPIO12 SCLK assignments. Do not change W01 pin assignments to match library defaults.
- [x] Initialize LovyanGFX and LVGL using the proven spike setup. Add a nonblocking initialization
  failure report; display failure must not create an indefinite wait for USB serial. The build
  confirms compilation; physical initialization remains for Phase 5.
- [x] Implement the signed, zero-referenced gauge page. Show synthetic pressure and `inH2O`, an
  animated indicator, status, and colored regions from the shared demo-band configuration. Label
  endpoints and zero without forcing zero to the dial midpoint.
- [x] Implement the 30-second trend page from the session's one-second, 30-entry history. Show the
  actual partial-history count after startup and do not fabricate points to fill the chart.
- [x] Implement the current-run minimum/maximum page from the same session state. Identify the
  page and retain values/history/extrema while navigating.
- [x] Apply the normal, warning, and danger treatments and signed value formatting. The value sits
  above a compact circular hub during needle movement.
- [x] Use LVGL animation and display servicing without blocking button polling or history sampling.
  Render on button events/sample boundaries and service LVGL each loop pass. No DMA, double
  buffering, or fixed frame-rate infrastructure was added.

  **Build evidence:** `platformio.exe run -e n16r8` passed on 2026-10-30 with Espressif32 8.0.0,
  Arduino-ESP32 3.3.12, LVGL 9.2.2, and LovyanGFX 1.2.35. The latest image uses 92,820 bytes RAM
  (28.3%) and 1,077,651 bytes flash (16.4%). This verifies compilation and size only; the refinements
  have not yet been uploaded for another visual review. The host test invocation from regular
  PowerShell could not find GCC/G++; the previously recorded UCRT64 suite passed 12/12 before this
  display-only phase.

## Phase 5: Integrated N16R8 verification

Initial physical review on 2026-10-30 confirmed the gauge responds to demo values and the trend
trace renders; the current-run min/max page looked acceptable. The first trend refinement improved
its numeric context, but its manually indexed scale labels were wrong: `-8` was attached to the
`-10` tick and `-3` was half a unit off. The latest implementation derives an evenly spaced tick
step from the visible span and all four boundaries, so each boundary is a native LVGL tick. Labels
are generated from tick pressure values rather than hand-counted indices, with selected two-unit
context labels. The colored ribbon sections continue to use the same boundaries. The trend keeps
its threshold shading and now includes the current reading/status and compact history context.
On 2026-10-30, physical review of the latest uploaded build confirmed the gauge labels align with
the colored bands and the display looks good. Follow-up checks verified page cycling, one-second
history sampling, and physical trend rollover. Exact threshold behavior is covered by host tests;
visual agreement at the exact transition values remains to be confirmed if needed.
On 2026-10-31, the display and gestures were reported responsive with the monitor open and closed,
and opening the monitor did not reset the page. Clicks remained intermittent: nine of ten deliberate
quarter-second presses registered in one series; 17 or 18 of 20 registered in a later series.
Temporary instrumentation measured maximum gaps between button polls of 664,714 us, 392,204 us,
and 398,280 us. In the two detailed timing reports, LVGL servicing accounted for 656,737 us and
381,836 us respectively; button polling, gesture handling, sampling, and sample-triggered rendering
each took less than 7 ms. This identifies synchronous LVGL service as the source of the long polling
gaps. The firmware now moves button polling to a higher-priority FreeRTOS task and queues gestures
for handling by the main loop. The subsequent physical test confirmed the change: more than 50
deliberate click gestures registered without a miss.
The captured event logs also show normal single-click values; double-click values classified as both
warning and danger within the synthetic span; a long press changing Gauge to Trend; and the history
reaching 30/30. A later physical check confirmed the complete Gauge → Trend → Min/Max → Gauge cycle
and that extrema remained intact on the min/max page. The physical trend graph also showed old
samples being replaced by the new reading over approximately 30 seconds while the history remained
at 30/30, confirming rollover behavior.

- [x] Run the normal `n16r8` PlatformIO build and upload; record dependency versions and commands.
  On 2026-10-31, `platformio.exe run -e n16r8` passed with Espressif32 8.0.0, Arduino-ESP32 3.3.12,
  LVGL 9.2.2, and LovyanGFX 1.2.35; the N16R8 was then uploaded for the physical checks below.
- [x] Verify all three pages on the physical GC9A01 and exercise every button gesture, including
  the revised scale labels and trend layout. The gauge labels align with the colored bands and the
  display was reported to look good. Gauge → Trend → Min/Max → Gauge page cycling worked, and the
  min/max values remained intact across navigation. More than 50 deliberate clicks registered
  without a miss after moving button polling to a separate FreeRTOS task. Double-clicks generated
  warning and danger values, and long-press navigation worked. Boundary color agreement remains an
  open check.
- [ ] Verify a click produces a new value in the normal demo band and a double-click produces a
  warning/danger value inside the visual span. **Partial result:** captured logs show normal click
  values and warning/danger double-click values within the synthetic span. Confirm on-screen
  classification and colored sections at and around all four boundaries.
- [x] Verify the trend records the current value on one-second ticks independently of the active
  page, retains only the latest 30 samples after rollover, and begins with an honest partial
  history. The history counter advanced at one-second cadence, logs showed an honest partial count
  before reaching `30/30`, and physical testing confirmed older graph samples were replaced by the
  new reading over approximately 30 seconds while the count stayed full.
- [x] Verify current-run extrema follow accepted synthetic readings and are not reset by page
  changes. Verify the long press cycles gauge → trend → min/max → gauge without altering session
  data. Captured event logs show extrema tracking synthetic readings; the physical min/max page
  displayed those extrema, and the full page cycle preserved them.
- [ ] Verify button polling, display service, history timing, and USB diagnostics remain responsive
  together. **Partial result:** monitor-closed responsiveness was reported restored after setting
  the USB CDC transmit timeout to zero and removing the heartbeat; no display reset occurred when
  stopping the monitor in the latest test. Verify gesture/event messages with the monitor open and
  confirm the one-second history cadence and 30-sample rollover with it closed. **One-second
  cadence and rollover confirmed** by the trend history counter and physical observation; repeat
  the monitor-closed responsiveness check with the latest button-polling-task build.
- [ ] Record manual hardware results separately from host-test results. Do not claim sensor,
  ADS1115, calibration, electrical validation, production alarm behavior, XIAO, or vehicle behavior.

## Phase 6: Documentation and closeout

- [ ] Update the firmware README and development guide with actual dependency/source revisions,
  build, host-test, upload, monitor, and host-toolchain instructions.
- [ ] Record BetterButton provenance/license, build and test results, host platform/compiler
  versions, N16R8/display setup, physical observations, and any deviations from the approved
  architecture.
- [ ] Confirm all displayed ranges and thresholds are labeled synthetic/demo-only; preserve the
  calibration-defined no-extrapolation requirement for future sensor acquisition.
- [ ] Update the M01 feature, architecture, plan, tasks, and `docs/status.md` only where actual
  implementation or verification changes project status. Keep unresolved items and unverified
  behavior explicit.
- [ ] Run `python tools/check-repository.py` and `git diff --check`. Review tracked, modified, and
  new files without changing Git staging or creating a commit.

## Wrap-up

- [ ] Review the complete M01 diff, including new files and source licenses. Confirm the exploratory
  spike remains reference material and there is only one production firmware application.
- [ ] Confirm the default `n16r8` firmware builds, host tests pass on each claimed platform, and the
  physical display/button checks have recorded outcomes. Mark M01 complete only for work actually
  demonstrated; leave native Linux or hardware checks open if not performed.
