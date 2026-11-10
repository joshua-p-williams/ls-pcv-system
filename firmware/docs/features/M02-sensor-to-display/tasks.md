# Tasks: Live Sensor-to-Display Measurement Path

This checklist expands the approved [M02 plan](plan.md). Complete and verify one phase at a time;
record discoveries in the relevant M02 document and hardware measurement records. A completed
software task does not imply that the corresponding physical behavior has been measured.

## Phase 1: Shared measurement core

- [x] Add shared measurement quality, source, pressure, history, zero-state, and immutable snapshot
  types to `gauge-core`, with documented units and validity semantics.
- [x] Implement the configurable piecewise pressure conversion and reject invalid profiles or
  voltages outside the active domain without extrapolation.
- [x] Integrate the existing M01 pressure classification boundaries and equality behavior with
  the shared valid-measurement model.
- [x] Implement timestamped measurement tracking, valid-sample extrema, one-second trend slots,
  explicit invalid gaps, and rollover behavior.
- [x] Implement the zero-capture algorithm as a parameterized core function/state machine; do not
  choose physical stability or plausibility limits before bench noise and sensor observations exist.
- [x] Adapt the M01 synthetic source and display snapshot to the shared data contract without
  changing its demo gesture behavior or implying synthetic readings are physical measurements.
- [x] Add Unity host tests for family reference points, polarity, rounding, profile/domain edges,
  exact band boundaries, invalid states, history/extrema, zero-capture outcomes, and demo behavior.
- [x] Run `pio test -e native`; record the command and result.
- [x] Update nearby code comments and documentation so the measurement units, assumptions, and
  invalid-data behavior are understandable without Arduino background.

Phase 1 verification: `pio test -e native` passed all 20 tests. The additional N16R8 integration
build, `pio run -e n16r8`, succeeded.

## Phase 2: Checked ADS1115 adapter and source

- [x] Implement the small project-owned ADS1115 register adapter over `Wire` for single-ended A0;
  document register choices against the preserved TI datasheet.
- [x] Check every I2C transaction result and return explicit initialization, bus, conversion, and
  timeout outcomes to the acquisition layer.
- [x] Configure the approved `+/-6.144 V` PGA range and initial single-shot `128 SPS` candidate;
  bound readiness polling and retain a clear fresh-conversion boundary.
- [x] Convert signed ADS1115 result codes to microvolts with tested sign extension and scaling.
- [x] Add a fake-I2C seam and tests for configuration, start/readiness/read sequence, sign/scaling,
  bus errors, initialization failure, and bounded timeout.
- [x] Implement the acquisition source so it owns ADC/I2C access, updates measurement state before
  publishing a snapshot, and never calls LVGL or performs routine blocking serial writes.
- [x] Build `pio run -e n16r8`; record the command and result.
- [x] Confirm the existing M01 composition does not initialize or access the ADC path. A separate
  `n16r8_demo` environment was not introduced in this phase.
- [x] Consider Adafruit ADS1X15 as a secondary implementation reference. No implementation detail
  beyond TI's register specification was needed; no runtime library dependency was added.

Phase 2 verification: `pio test -e native` passed all 30 tests across the ADS1115 and gauge-core
suites. `pio run -e n16r8` succeeded. These checks verify software behavior and target compilation;
the physical I2C bus, module identity, analog input, and sensor remain unverified until Phase 3.

## Phase 3: ADC voltage acquisition on the assembled bench

- [x] Add the `n16r8_adc` bench target. It polls the selected ADS1115 in its own
  FreeRTOS task and reports raw conversion codes and derived A0 microvolts at a bounded rate;
  it does not report the battery test source as pressure. The existing `n16r8` M01 demo target
  remains unchanged.
- [x] Document the build/upload/monitor procedure and distinguish calculated ADC voltage from an
  independent multimeter reading.
- [x] Run the native host suite, build `n16r8_adc`, and rebuild `n16r8` to check the ADC target
  remains separate from the M01 display demo.
- [x] Review W01 and the current board configuration for ADC supply, common ground, translated I2C,
  selected address, and A0 before applying a test input.
- [x] Upload `n16r8_adc` with the FTP signal disconnected; verify the ADS1115 initializes and
  conversion results are fresh.
- [x] Measure ADC VDD and ground at the module and confirm the ADC is powered before connecting an
  A0 test source.
- [x] Select multiple battery-derived sources only after measuring them; exclude any source whose
  A0-node voltage could fall outside the actual powered ADC ground-to-VDD range.
- [x] Measure each voltage at the A0-side test node and shared ground, then compare multimeter
  voltage, raw ADS code, configured gain/rate, converted microvolts, and repeated-reading spread.
- [x] Record battery/source type, measured node voltage, ADC configuration/raw readings, derived
  voltage, meter resolution/accuracy if known, and observations in a bench record.
- [x] Confirm readings move in the expected direction and track the measured voltage sufficiently
  for an acquisition check; do not claim pressure calibration or absolute ADC accuracy.
- [x] If behavior conflicts with the ADS1115 working assumption, stop pressure interpretation and
  document the observed discrepancy and its resolution before continuing.

Phase 3 software readiness: `pio test -e native` passed all 31 host tests; `pio run -e n16r8_adc`
and `pio run -e n16r8` both succeeded. The repository checker and `git diff --check` also passed.
Bench observations on 2026-11-02 are recorded in the [ADC voltage test record](../../../../hardware/components/adc/bench-voltage-observations-2026-11-02.md)
and [grounded-input record](../../../../hardware/components/adc/bench-grounded-input-observation-2026-11-02.md).
USB-powered ADC VDD measured 4.75 V at the module; the ADS1115 returned fresh conversions at the
selected `0x48` address. AAA and CR123A inputs at the A0 node tracked multimeter readings at about
1.60 V and 3.32 V, with repeated ADC values varying by at most one count. A grounded input produced
repeated zero readings. The initial grounded-input capture produced occasional `raw_code=-1`
results, leading to a narrowly bounded zero-offset handling change: exactly `-1` now converts to
0 µV while the raw code is retained, and codes of `-2` or lower remain ADC faults. After uploading
that change, the retest recorded 137 consecutive zero-voltage samples, including 13 `raw_code=-1`
results, without `adc_error` or `i2c_error`; see the [grounded-input record](../../../../hardware/components/adc/bench-grounded-input-observation-2026-11-02.md).
The input drifted when left open. These results verify this voltage-acquisition bench check only;
the FTP sensor, pressure conversion, RC response, vehicle operation, and complete W01 harness
remain unverified.

## Phase 4: FTP sensor and provisional live pressure path

- [x] Reconnect the FTP sensor through the documented RC filter and confirm sensor/ADC nominal 5 V
  and common-ground conditions before interpreting readings.
- [x] Measure and record sensor output and ADS A0 voltage at atmospheric equalization, along with
  supply voltage, raw code/configuration, and observed stability.
- [x] Compare polarity and nominal conversion with the published family reference model; label the
  result provisional and preserve the measured observations separately from assumptions.
- [x] Integrate live acquisition snapshots into the Gauge, Trend, and Min/Max pages; keep page and
  LVGL state owned by the application loop.
- [x] Implement distinct unavailable, stale, I2C/ADC error, and out-of-domain display states; suppress
  the current numeric pressure/classification for invalid data.
- [ ] Confirm extrema use each accepted sample, trend slots use the one-second cadence, and stale
  values are not repeated as fresh samples.
- [x] On the device, confirm the three pages show the live value, colored status, trend, and extrema;
  create an out-of-domain reading and confirm it is reported as invalid rather than as a current
  pressure value.
- [x] Confirm the trend shows a gap during an out-of-domain interval.
- [ ] Safely simulate an ADC or I2C acquisition fault and confirm no stale number remains displayed
  as current.
- [x] Exercise the display and button with the serial monitor closed; verify diagnostics are not
  required for timely acquisition, gestures, or screen refresh.
- [x] Record observed behavior, signal voltages, and limitations; do not characterize this check as
  a calibrated pressure result.

The first atmospheric sensor/ADC check is recorded in the [bench observation](../../../../hardware/components/adc/bench-ftp-atmospheric-observation-2026-11-02.md). Sensor supply measured 4.76 V, sensor output measured 1.474 V, and A0 measured 1.473 V. Sixteen consecutive ADC reports remained at raw code 7853 except one 7854 reading; pressure remains untested by this diagnostic firmware. The observed atmospheric output is within the published family reference range, but pressure direction and slope still require controlled characterization.

Phase 4 software implementation now routes the live ADS1115 source to the existing three-page
display. `n16r8` is the live configuration; `n16r8_demo` preserves the M01 synthetic interaction,
and `n16r8_adc` remains the voltage-only diagnostic. The live source retains the documented family
zero reference of 1.500 V. The atmospheric measurement was not substituted as calibration; the
display may therefore show a small provisional offset until the confirmed Settings zero workflow
is implemented in Phase 6. A valid reading becomes visually stale after 1,500 ms, and fault states
remove the current numeric pressure while accepted extrema remain historical. Host tests passed
31/31; all three ESP32 configurations compiled. Initial board checks confirmed live page behavior,
out-of-domain indication, trend gaps, and operation with the serial monitor closed; the limits of
those qualitative observations are recorded below.

Phase 4 bench verification (2026-11-03): serial output at atmospheric pressure reported raw codes
7847–7848 and 1.471313–1.471500 V at A0; the display showed the family-default estimate of
+0.1 inH2O in the danger band. The Gauge, Trend, and Min/Max pages showed the live reading, trend,
and current-run extrema. Applied vacuum and pressure moved the displayed reading in the expected
directions; an out-of-domain interval produced an out-of-range message and a gap in the trend.
Display and button operation continued with the serial monitor closed. This was a qualitative
functional check: applied pressure values and an independent pressure reference were not recorded,
so it does not validate conversion accuracy or warning/danger thresholds. The ADC/I2C acquisition
fault presentation and detailed sample/extrema cadence checks remain open.

## Phase 5: Controlled pressure and vacuum characterization

- [ ] Identify the purchased digital manometer's visible make/model, units, range, resolution, zero
  behavior, and available accuracy documentation; retain unknown specifications as unknown.
- [ ] Build or document a controllable low-pressure fixture with a common reference connection,
  gradual application, isolation/bleed/equalization, and a liquid barrier protecting the sensor.
- [ ] Use the U-tube vertical difference between liquid levels as the primary low-pressure
  reference; use the digital unit as a corroborating indication, not an assumed calibration
  standard.
- [ ] Keep pressure/vacuum within the sensor and instruments' actual limits; do not connect the HVAC
  pump directly to the sensor or infer a pressure from ADC saturation.
- [ ] Record repeated atmospheric, positive, and vacuum points in both approach directions where
  safe, including pressure reference, water-column height difference, meter display/mode, sensor
  supply/output, A0 voltage, ADC setup/raw code, converted pressure, and stability notes.
- [ ] Preserve raw observations unchanged and document how any derived transfer relationship or
  valid measurement domain was calculated.
- [ ] Compare observed points with the family model; update the active conversion profile and host
  tests only when the evidence supports the change.
- [ ] Update M02 research and the appropriate component/test records with the method, instrument
  limits, results, and unresolved measurement limits.

## Phase 6: Confirmed zero workflow and persistent storage

- [x] Use observed atmospheric-chain noise and sensor results to select and document the finite
  capture window, stability statistic/limit, and plausibility bounds.
- [x] Implement the four-page live navigation and Settings `Set zero` item using the approved
  long-press, click, double-click, Cancel-default, and inactivity-cancel behavior.
- [x] Implement the prompt and capture status text stating engine off/cold and oil cap open; do not
  imply firmware can detect those conditions or atmospheric equalization.
- [x] Send an explicit confirmed capture command to the acquisition task; prevent duplicate requests
  while capture or save is active and keep sampling nonblocking.
- [x] Persist only an accepted zero reference in a versioned Preferences/NVS record; validate the
  loaded record and otherwise use the documented family default.
- [x] Report save success only when storage confirms success; expose capture rejection, invalid
  stored data, or write failure without changing the prior valid calibration silently.
- [x] Add host tests for capture criteria, stable/unstable windows, plausibility, record
  version and value checks, fallback, and persistence success/failure through a fake store.
- [x] Verify offset-only behavior: changing zero does not change either pressure slope or status
  boundaries.
- [x] On device, verify Cancel exits the zero workflow without saving a new reference.
- [x] On device, verify leaving the confirmation prompt untouched for 30 seconds returns to the
  Settings menu with Cancel selected and leaves the stored reference unchanged. The `SET ZERO`
  menu itself has no inactivity timeout.
- [x] On device, verify an unstable capture reports rejection and does not report a successful
  save.
- [ ] Verify a storage write failure never reports success; host tests cover the fake-store failure
  path, but physical NVS failure has not been induced.
- [x] On device, confirm a saved zero is applied to live pressure conversion after a power cycle.
- [ ] Record the capture conditions and observed result without describing one-point zero as
  multi-point calibration or accuracy validation.

The initial capture policy uses 64 consecutive ADS1115 samples, accepts a candidate only when all
samples are within the documented family atmospheric interval of 1.3–1.7 V, and limits the window's
peak-to-peak spread to 2.0 mV. At the selected ±6.144 V ADC range, one code is 187.5 µV. The
atmospheric bench observations varied by one code within each session; the two recorded sessions
spanned six codes (about 1.125 mV). The 2.0 mV limit allows modest headroom over that observed
session-to-session spread while rejecting a changing signal. Sixty-four conversions give a short,
bounded capture window and their arithmetic mean reduces quantization variation. These are
provisional acceptance settings based on the current sensor/ADC observations, not validated
measurement-accuracy limits. The 1.3–1.7 V bounds are inherited from the advertised GM family
reference and do not authenticate the aftermarket sensor. The manometer/U-tube work remains
necessary for pressure conversion and threshold validation; one-point zero changes only the
atmospheric offset.

Host unit tests pass 36/36. Both `pio run -e n16r8` and `pio run -e n16r8_demo` build successfully;
the demo retains its original three-page M01 behavior. Device checks have since confirmed Cancel,
the 30-second confirmation timeout, unstable-capture rejection, and saved-zero use after reboot.
An actual NVS write failure has not been induced on hardware.

The first Settings-page device review showed a storage error with no zero record saved. The initial
adapter attempted to open a not-yet-created Preferences namespace read-only, so the first-use path
was misreported as an NVS error instead of using the family default. The adapter now opens or
creates the empty namespace read-write without writing a zero value, then treats a missing `zero`
key as the expected default case. The display also shows the active zero voltage and highlights the
selected `Set zero` action. Recheck the first-use screen on the board; a genuine NVS open/write error
still remains visible as an error and must not be reported as a saved setting.

A subsequent power-cycle check showed that the saved voltage remained visible in Settings, while
the live gauge returned to `+0.1 inH2O`. The record was being loaded, but the boot path did not copy
the loaded reference into the pressure-conversion profile; newly captured references did update
that profile. The acquisition source now synchronizes the active reference during initialization,
and a host regression test confirms that a reading at the saved zero converts to zero after boot.
After uploading the fix, the saved zero remained active after reboot and the gauge returned to zero;
the hardware power-cycle check is complete.

On-device Settings checks confirmed that Cancel exits without saving, and that a moving signal is
rejected with `ZERO NOT SAVED` / `Reading changed during capture. Retry when steady.` / `CAPTURE
REJECTED`. This demonstrates the capture-stability guard and its user-facing rejection state. The
inactivity timeout and an actual storage-write failure remain unverified on hardware.

## Phase 7: Integrated live and demo verification

On 2026-11-03, `pio test -e native` passed all 36 tests, and both `pio run -e n16r8` and
`pio run -e n16r8_demo` succeeded. The native command was run from the MSYS2 UCRT64 shell so GCC
was available; target builds used the installed PlatformIO executable from PowerShell. The demo
was exercised on the board: synthetic click/double-click readings and long-press page changes
worked without relying on the live sensor/ADC path. The `n16r8` live build was then uploaded again
and reverified as working.

- [x] Run the full native suite with `pio test -e native` and record the result.
- [x] Build both configurations with `pio run -e n16r8` and `pio run -e n16r8_demo`; record the
  commands and results.
- [x] Upload the live environment and verify the complete sensor → RC filter → ADS1115 → pressure
  conversion → snapshot/history → display path.
- [x] Verify the four live pages, changing sensor readings, status presentation, and invalid/out-of-
  range indication on the display.
- [ ] Exercise a stale reading or ADC/I2C fault and verify the display does not leave an old numeric
  pressure presented as current.
- [x] Confirm an out-of-domain interval creates a gap in the trend and the live Min/Max page shows
  the current-run extrema.
- [x] Verify on the device that trend history advances at the one-second cadence.
- [ ] Verify extrema capture remains independent of display smoothing and serial-monitor state.
- [x] Confirm buttons and display remain responsive with the USB serial monitor both open and
  closed; diagnostics are optional for normal operation.
- [x] Upload or otherwise exercise the demo build and confirm synthetic source identity, M01 gesture
  behavior, and independence from the physical sensor/ADC path.
- [ ] Reconcile observed deviations into the relevant feature, research, architecture, or plan
  document before marking this phase complete.

## Wrap-up

- [ ] Confirm each phase has its specified automated, build, electrical, pressure, and on-device
  evidence recorded separately; leave unperformed checks unchecked.
- [ ] Preserve raw battery and pressure observations; keep derived conversions separate and identify
  their source data and method.
- [ ] Update affected component, measurement-system, and project-status references if M02 findings
  change a maintained design decision or open question.
- [ ] Review documentation links, units, evidence labels, source attribution, and code comments for
  consistency with the implemented behavior.
- [ ] Run `python tools/check-repository.py` with the documented repository-check dependencies and
  run `git diff --check`; review untracked files separately.
- [ ] Draft a concise commit message for human review; do not create a commit as part of task
  completion.
