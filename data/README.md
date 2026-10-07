# Calibration and test records

No measurements have been imported yet. Create `calibration/` and `logs/` with their first real records; do not populate them with invented example readings.

See [A03: pressure measurement signal chain](../hardware/architecture/measurement-system.md) for the distinction between raw counts, calibrated pressure, display values and calibration/zero changes.

## Storage conventions

- Keep original acquisition files unchanged, including native logger formats.
- Use CSV for portable tabular exports with explicit units in column names. Preserve timestamps or elapsed time, and raw ADC counts alongside calibrated pressure when available.
- Give each run a stable ID such as `2026-10-05-run-001`; use ISO dates and record timezone for wall-clock timestamps.
- Use a neighboring Markdown file with the same stem for setup, provenance, and observations.
- Store processed results separately with links to raw inputs and the script or calculation used. Do not overwrite original data during recalibration.
- Missing values are missing, not zero. Explain blanks, dropped samples, fault flags, and unavailable channels.

## Calibration metadata

Record sensor ID/exact part, terminal mapping, date, pressure reference and sign convention, supply/reference voltage, ADC module identity/resolution, gain, conversion rate/mode, channel/address, conditioning component values, hardware/firmware revision, atmospheric zero procedure, applied pressures, raw readings, repetitions, fitted model/coefficient units, residuals, and validity range. Identify which calibration a vehicle run uses.

## Vehicle-run metadata

Record run ID, date/timezone, vehicle configuration, source commit when available, restrictor measured bore, hose routing, relief configuration and test record, pressure tap, calibration ID, sample rate, filtering/peak method, operating conditions, and observations. Include actual channels and their units; RPM/MAP/TPS are optional if not recorded.

Suggested pressure channel names: `elapsed_ms`, `raw_adc_counts`, `adc_input_voltage_v`, `sensor_voltage_v`, `pressure_inh2o`, `display_pressure_inh2o`. Distinguish the scaled voltage at the ADC input from a sensor voltage reconstructed using the divider ratio. This is a naming guide, not a required logger schema or fabricated dataset. A voltage column requires a known conversion/reference; a computed voltage is not an independently measured value.

Follow the [test plan](../docs/testing/test-plan.md) and keep assumptions separate from evidence.
