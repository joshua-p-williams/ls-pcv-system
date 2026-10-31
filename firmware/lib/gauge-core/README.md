# Gauge core

`gauge-core` holds M01's pressure-band rules, synthetic demonstration source, gauge session state,
trend history, extrema, and temporary gesture mapping. It has no dependency on Arduino, the ESP32,
LVGL, LovyanGFX, or a physical device, so PlatformIO can exercise these rules on a host computer.

Pressure values use signed tenths of `inH2O`. For example, `-55` means `-5.5 inH2O`; this is a
display representation for synthetic movement, not a claim about sensor resolution or accuracy.
The `DemoBandConfig` is the single source for the visible `-14.0` to `+4.5 inH2O` span and the
temporary lower/upper warning and danger boundaries. These values exist to demonstrate UI states;
they are not measured engine limits or production alarm settings.

`GaugeSession` accepts only values inside that demo span. It derives the status, tracks extrema,
and stores one current value per elapsed one-second sample interval in a 30-slot rolling history.
The snapshot is chronological and may contain fewer than 30 samples after startup. Gesture handling
changes synthetic values or the active page; gestures do not directly create trend samples. This
logic is intentionally separate from the later calibrated sensor and ADC contract.

Build and run the native tests from `firmware/` with `pio test -e native`. See the [development
environment guide](../../docs/development-environment.md) for the host GCC prerequisite and setup.
