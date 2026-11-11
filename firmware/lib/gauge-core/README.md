# Gauge core

`gauge-core` holds hardware-independent pressure conversion, validity and status rules, measurement
tracking, trend history, extrema, zero-capture evaluation, and the M01 synthetic demonstration.
It has no dependency on Arduino, the ESP32, FreeRTOS, I2C, NVS, LVGL, LovyanGFX, or a physical
device, so PlatformIO can exercise these rules on a host computer.

Pressure values use signed tenths of `inH2O`. For example, `-55` means `-5.5 inH2O`; this is a
display representation for synthetic movement, not a claim about sensor resolution or accuracy.
`PressureBandThresholds` holds the shared lower/upper warning and danger boundaries. Classification
does not decide whether a sensor voltage is valid; the conversion profile checks its own input
domain first. `DemoBandConfig` adds M01's visible `-14.0` to `+4.5 inH2O` span for synthetic value
generation and display. Those demo limits and status bands are illustrative guidance, not measured
engine limits or production alarm settings.

`PressureCalibrationProfile` uses integer microvolts and rational slopes to convert the provisional
family reference points into signed tenths of `inH2O`. The default `1.5 V`, `+4.5 inH2O`, and
`-14 inH2O` anchors are a piecewise-linear starting model, not a manufacturer transfer curve or
measured calibration. Inputs outside the profile's inclusive voltage domain are rejected instead
of extrapolated. Changing the atmospheric zero does not change either slope.

`MeasurementTracker` separates measurement quality from pressure status. Only a valid pressure is
classified or included in extrema. Each one-second trend slot contains a validity flag, pressure,
and monotonic timestamp; a slot without a fresh valid observation is a gap rather than a repeated
old live value. The synthetic M01 source intentionally repeats its held demo state through elapsed
slots, preserving the demo's prior behavior. The tracker handles wraparound and retains the newest
30 chronological slots.
`GaugeDisplaySnapshot` adds page and display-range state to a detached measurement snapshot.

`ZeroReferenceCapture` evaluates a bounded set of microvolt samples using caller-supplied
plausibility and peak-to-peak criteria. It intentionally defines no physical stability limit; those
criteria must come from measured noise and the active sensor profile. Its mean is only a candidate
offset, not a multi-point calibration.

The M01 `GaugeSession` and `DemoPressureSource` adapt the synthetic gestures and readings to these
shared contracts. Demo values are explicitly tagged `synthetic`, and the demo's display span is
checked separately from the shared classifier. Button gestures still change the synthetic value or
page; timer sampling, not the gesture event itself, records trend points.

Build and run the native tests from `firmware/` with `pio test -e native`. See the [development
environment guide](../../docs/development-environment.md) for the host GCC prerequisite and setup.
