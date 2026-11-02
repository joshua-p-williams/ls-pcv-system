# Tasks: Live Sensor-to-Display Measurement Path

This checklist expands the approved [M02 plan](plan.md). Complete and verify one phase at a time;
record discoveries in the relevant M02 document and hardware measurement records. A completed
software task does not imply that the corresponding physical behavior has been measured.

## Phase 1: Shared measurement core

- [ ] Add shared measurement quality, source, pressure, history, zero-state, and immutable snapshot
  types to `gauge-core`, with documented units and validity semantics.
- [ ] Implement the configurable piecewise pressure conversion and reject invalid profiles or
  voltages outside the active domain without extrapolation.
- [ ] Integrate the existing M01 pressure classification boundaries and equality behavior with
  the shared valid-measurement model.
- [ ] Implement timestamped measurement tracking, valid-sample extrema, one-second trend slots,
  explicit invalid gaps, and rollover behavior.
- [ ] Implement the zero-capture algorithm as a parameterized core function/state machine; do not
  choose physical stability or plausibility limits before bench noise and sensor observations exist.
- [ ] Adapt the M01 synthetic source and display snapshot to the shared data contract without
  changing its demo gesture behavior or implying synthetic readings are physical measurements.
- [ ] Add Unity host tests for family reference points, polarity, rounding, profile/domain edges,
  exact band boundaries, invalid states, history/extrema, zero-capture outcomes, and demo behavior.
- [ ] Run `pio test -e native`; record the command and result.
- [ ] Update nearby code comments and documentation so the measurement units, assumptions, and
  invalid-data behavior are understandable without Arduino background.

## Phase 2: Checked ADS1115 adapter and source

- [ ] Implement the small project-owned ADS1115 register adapter over `Wire` for single-ended A0;
  document register choices against the preserved TI datasheet.
- [ ] Check every I2C transaction result and return explicit initialization, bus, conversion, and
  timeout outcomes to the acquisition layer.
- [ ] Configure the approved `+/-6.144 V` PGA range and initial single-shot `128 SPS` candidate;
  bound readiness polling and retain a clear fresh-conversion boundary.
- [ ] Convert signed ADS1115 result codes to microvolts with tested sign extension and scaling.
- [ ] Add a fake-I2C seam and tests for configuration, start/readiness/read sequence, sign/scaling,
  bus errors, initialization failure, and bounded timeout.
- [ ] Implement the acquisition source so it owns ADC/I2C access, updates measurement state before
  publishing a snapshot, and never calls LVGL or performs routine blocking serial writes.
- [ ] Build `pio run -e n16r8`; record the command and result.
- [ ] Confirm the demo composition does not initialize or access the ADC path; build
  `pio run -e n16r8_demo` if its environment is introduced in this phase.
- [ ] Use Adafruit ADS1X15 as a secondary implementation reference if needed; record any relevant
  comparison without adding it as a runtime dependency unless architecture is explicitly revised.

## Phase 3: ADC voltage acquisition on the assembled bench

- [ ] Review W01 and the current board configuration for ADC supply, common ground, translated I2C,
  selected address, and A0 before applying a test input.
- [ ] Upload the live N16R8 build with the FTP signal disconnected; verify the ADS1115 initializes
  and conversion results are fresh.
- [ ] Measure ADC VDD and ground at the module and confirm the ADC is powered before connecting an
  A0 test source.
- [ ] Select multiple battery-derived sources only after measuring them; exclude any source whose
  A0-node voltage could fall outside the actual powered ADC ground-to-VDD range.
- [ ] Measure each voltage at the A0-side test node and shared ground, then compare multimeter
  voltage, raw ADS code, configured gain/rate, converted microvolts, and repeated-reading spread.
- [ ] Record battery/source type, measured node voltage, ADC configuration/raw readings, derived
  voltage, meter resolution/accuracy if known, and observations in a bench record.
- [ ] Confirm readings move in the expected direction and track the measured voltage sufficiently
  for an acquisition check; do not claim pressure calibration or absolute ADC accuracy.
- [ ] If behavior conflicts with the ADS1115 working assumption, stop pressure interpretation and
  document the observed discrepancy and its resolution before continuing.

## Phase 4: FTP sensor and provisional live pressure path

- [ ] Reconnect the FTP sensor through the documented RC filter and confirm sensor/ADC nominal 5 V
  and common-ground conditions before interpreting readings.
- [ ] Measure and record sensor output and ADS A0 voltage at atmospheric equalization, along with
  supply voltage, raw code/configuration, and observed stability.
- [ ] Compare polarity and nominal conversion with the published family reference model; label the
  result provisional and preserve the measured observations separately from assumptions.
- [ ] Integrate live acquisition snapshots into the Gauge, Trend, and Min/Max pages; keep page and
  LVGL state owned by the application loop.
- [ ] Render unavailable, stale, I2C/ADC error, and out-of-domain conditions distinctly and suppress
  current numeric pressure/classification for invalid data.
- [ ] Confirm extrema use each accepted sample and trend slots use the one-second cadence, with
  gaps for invalid intervals and no repeated stale values.
- [ ] Exercise the display and button with the serial monitor closed; verify diagnostics are not
  required for timely acquisition, gestures, or screen refresh.
- [ ] Record observed behavior, signal voltages, and limitations; do not characterize this check as
  a calibrated pressure result.

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

- [ ] Use observed atmospheric-chain noise and sensor results to select and document the finite
  capture window, stability statistic/limit, and plausibility bounds; do not invent these constants
  before measurement.
- [ ] Implement the four-page live navigation and Settings `Set zero` item using the approved
  long-press, click, double-click, Cancel-default, and inactivity-cancel behavior.
- [ ] Implement the prompt and capture status text stating engine off/cold and oil cap open; do not
  imply firmware can detect those conditions or atmospheric equalization.
- [ ] Send an explicit confirmed capture command to the acquisition task; prevent duplicate requests
  while capture or save is active and keep sampling nonblocking.
- [ ] Persist only an accepted zero reference in a versioned Preferences/NVS record; validate the
  loaded record and otherwise use the documented family default.
- [ ] Report save success only when storage confirms success; expose capture rejection, invalid
  stored data, or write failure without changing the prior valid calibration silently.
- [ ] Add host tests for measured capture criteria, stable/unstable windows, plausibility, record
  version and value checks, fallback, and persistence success/failure through a fake store.
- [ ] Verify offset-only behavior: changing zero does not change either pressure slope or status
  boundaries.
- [ ] On device, verify Cancel and inactivity preserve stored data; verify a stable confirmed zero
  survives a power cycle; verify rejected capture or failed persistence never reports success.
- [ ] Record the capture conditions and observed result without describing one-point zero as
  multi-point calibration or accuracy validation.

## Phase 7: Integrated live and demo verification

- [ ] Run the full native suite with `pio test -e native` and record the result.
- [ ] Build both configurations with `pio run -e n16r8` and `pio run -e n16r8_demo`; record the
  commands and results.
- [ ] Upload the live environment and verify the complete sensor → RC filter → ADS1115 → pressure
  conversion → snapshot/history → display path.
- [ ] Verify all four live pages, initial status-band presentation, fresh-value behavior, and
  invalid/out-of-range/error presentation.
- [ ] Confirm one-second trend gaps and current-run extrema remain truthful during display work and
  that peak behavior does not depend on display smoothing or serial monitoring.
- [ ] Confirm buttons and display remain responsive with the USB serial monitor both open and
  closed; ensure diagnostics are optional and bounded.
- [ ] Upload or otherwise exercise the demo build and confirm synthetic source identity, M01 gesture
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
