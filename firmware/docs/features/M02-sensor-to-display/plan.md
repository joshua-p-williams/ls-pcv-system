# Plan: Live Sensor-to-Display Measurement Path

This is the fourth planning document for [M02](feature.md), following its approved
[research](research.md) and [architecture](architecture.md). It defines the order of
implementation and evidence collection. The task checklist belongs in `tasks.md` and will be
drafted after this plan is reviewed and approved.

## Approach

Build from the measurement model outward. First establish and test hardware-independent pressure,
validity, history, and zero-capture behavior. Then implement the narrow ADS1115 adapter and prove
voltage acquisition with safe measured electrical inputs before connecting the FTP sensor. Add the
sensor model and compare it with atmospheric and controlled pressure references. Finally integrate
the user-confirmed zero workflow, persistence, and live display pages, while keeping the M01 demo
build independently usable. At every stage, distinguish software test results from observations of
the assembled hardware.

Bench testing uses W01 as the connectivity authority. Do not connect the HVAC vacuum pump directly
to the sensor. Use a controllable low-pressure source, a visible reference, a means to isolate and
bleed/equalize the line, and a liquid barrier before applying pressure or vacuum. Keep stimulus
within the actual sensor and reference-instrument limits; the U-tube is the primary low-pressure
reference, and the purchased digital manometer is corroborating evidence until its specifications
are identified.

## Sequence

1. **Establish the shared measurement core.** Add the common measurement and display-facing data
   contracts, validity states, pressure conversion profile, classifier integration, history/trend
   handling, and zero-capture algorithm to `gauge-core`. Adapt the existing M01 demo to the shared
   display contract without changing its temporary gesture behavior. Keep conversion and state rules
   independent of Arduino, FreeRTOS, I2C, NVS, and LVGL.

   **Exit evidence:** native host tests cover family-reference conversion points, sign and rounding,
   valid-domain edges, exact status boundaries, invalid readings, history gaps/extrema, zero-capture
   acceptance/rejection, and demo regression. `pio test -e native` passes. The tests use explicit
   test vectors; they are not represented as physical measurements or calibrated sensor results.

2. **Implement the checked ADS1115 adapter and source.** Add the project-owned, single-ended A0
   register adapter with checked I2C operations, bounded single-shot readiness polling, code-to-
   microvolt conversion, and explicit error/timeout outcomes. Add a fake-I2C seam for host tests.
   Keep register choices tied to the TI datasheet and use Adafruit ADS1X15 as a secondary reference
   if register behavior or troubleshooting needs comparison.

   **Exit evidence:** tests exercise configuration, conversion start/readiness, conversion result
   sign extension and scaling, I2C failures, timeout, and initialization failure. The live firmware
   builds for `n16r8`; no pressure value is presented as valid when the driver reports an error or
   stale conversion.

3. **Verify ADC voltage acquisition on the assembled bench.** Build and upload the live environment
   with the FTP signal disconnected. Confirm the selected 5 V ADC supply, common ground, I2C
   communication, address/configuration, and A0 input path. Apply several measured battery-derived
   voltages at the A0-side test node, measuring at that node with the multimeter. Use only a source
   whose measured input remains between the powered ADC's actual ground and supply rails; do not
   connect an input to an unpowered ADC. Record source, test-node voltage, ADC raw code/configuration,
   converted voltage, and observed spread.

   **Exit evidence:** each safe input is reported consistently by the ADC, changes in the expected
   direction, and is reasonably consistent with the multimeter within the instruments' known
   resolution/accuracy limits. This is an acquisition and wiring check, not an absolute ADC
   calibration or pressure test. If results conflict with ADS1115 behavior, investigate the bus,
   configuration, or working module assumption before proceeding.

4. **Connect the FTP sensor and exercise the provisional pressure path.** Reconnect the sensor
   through the built RC filter. Confirm the sensor/ADC 5 V supply and measure the sensor output and
   A0 voltage at atmospheric equalization. Verify signal polarity and the provisional family
   conversion against the published reference model. Exercise the live gauge, trend, min/max, and
   fault presentation with the serial monitor closed; compare display behavior with acquisition
   diagnostics only when diagnostics are needed.

   **Exit evidence:** the live build produces fresh, plausible provisional values at atmospheric
   equalization; pressure conversion has the expected polarity; invalid, stale, ADC/I2C-error, and
   out-of-domain states suppress a misleading current numeric value; run extrema and one-second
   history update independently of display refresh. Record observed output as bench evidence, not
   as calibrated accuracy.

5. **Characterize the sensor against controlled pressure and vacuum.** Establish a safe fixture
   that applies and bleeds low pressure gradually, references the sensor and manometer to the same
   pressure, and prevents water entering the sensor. Use the U-tube's vertical liquid-level
   difference as the primary reference and record the digital manometer's identity, mode, units,
   resolution, and displayed reading as a cross-check. Collect repeated points in both directions,
   including atmospheric equalization and points within the planned sensor-family domain. Preserve
   raw observations and source/tool details before deriving a transfer relationship or accepted
   measurement domain.

   **Exit evidence:** a dated bench record contains reference pressure, direction/approach,
   sensor supply, sensor output and A0 voltage, ADS configuration/raw code, converted result, and
   stability/repeatability notes. Update the conversion profile and its tests only from the
   measured results, retaining the family model and its provenance as the fallback where
   appropriate. Do not infer saturation pressure from an ADC rail or claim certified accuracy for
   the instruments.

6. **Integrate the confirmed zero workflow and persistent storage.** Use measured atmospheric
   chain noise and the controlled bench observations to select and document a finite capture
   window, stability criterion, and plausibility bounds. Implement the Settings confirmation,
   acquisition-task capture, versioned Preferences/NVS record, fallback to the family default,
   and clear success/failure/rejection states. Verify the offset changes the reference only and
   leaves pressure slopes and warning/danger boundaries unchanged.

   **Exit evidence:** host tests cover the chosen capture criteria, record validation/version,
   missing or invalid-record fallback, and simulated storage failures. On-device checks show that
   cancel/inactivity leaves storage unchanged; a confirmed stable atmospheric capture reports
   success and survives power cycling; rejected/unstable capture or failed storage never reports
   success. The UI documents the required engine-off/cold and oil-cap-open condition and never
   performs automatic zeroing.

7. **Complete integrated live and demo verification.** Exercise the complete live path from sensor
   through filter, ADC, conversion, history, and display. Re-run the synthetic environment to ensure
   it remains independent of sensor hardware and retains the M01 behavior. Confirm button and
   display responsiveness with USB serial monitoring both active and closed, and confirm
   diagnostics cannot stall normal operation.

   **Exit evidence:** `pio test -e native`, `pio run -e n16r8`, and `pio run -e n16r8_demo` pass.
   The N16R8 live build displays the physical sensor path and all four live pages, identifies
   unavailable/fault/out-of-range readings without stale numeric pressure, and preserves peak and
   trend behavior during display work. The demo build still generates synthetic values and does
   not initialize or depend on the sensor/ADC path. Bench observations and any remaining limits are
   recorded in the relevant measurement and component records.

## Verification / definition of done

M02 is complete when the normal `n16r8` configuration uses fresh, validated ADS1115 readings from
the FTP sensor; the separate `n16r8_demo` configuration preserves the M01 synthetic demonstration;
and the shared math, validity, history, conversion, and zero-capture behavior are covered by passing
native tests. The live display presents pressure only when the measurement is valid and fresh,
maintains extrema and trend independently of LVGL refresh, and provides the approved four-page
navigation and confirmed persistent zero adjustment.

Record these evidence classes separately:

- **Automated:** native Unity tests, including fake-I2C driver behavior and core edge cases.
- **Build:** PlatformIO build results for both N16R8 environments.
- **Electrical bench:** rail, I2C, and battery-to-voltage comparison observations.
- **Pressure bench:** raw sensor and reference-instrument observations with fixture and instrument
  details.
- **On-device behavior:** display, button, error-state, trend/extrema, and zero-storage behavior,
  including checks with the serial monitor closed.

A successful host test or firmware build does not establish assembled electrical behavior,
pressure accuracy, sensor calibration, or engine-specific alarm limits. Keep the initial status
bands as documented guidance and preserve all raw measurement data. Update the M02 research or
architecture when actual evidence changes a working assumption or design.

## Risks / things to watch

- The purchased sensor is identified through an advertised GM-family cross-reference; the published
  service points are a provisional model, not a complete transfer curve or guaranteed aftermarket
  specification. Do not extrapolate beyond the characterized active domain.
- The ADS1115 product listing/BOM is the selected working assumption despite shared
  ADS1115/ADS1015 board silk. Unexpected raw-code width, scaling, address, or behavior is a concrete
  reason to revisit the device assumption before converting pressure.
- `+/-6.144 V` is an ADS conversion range, not permission for an input above its supply. Keep A0
  between actual ground and VDD and keep the ADC powered during input tests.
- The RC filter, sensor output impedance, module input, and sampling cadence affect dynamic
  response. Verify useful peak capture; do not equate ADC SPS or display refresh with end-to-end
  pressure bandwidth.
- The digital manometer's advertised range/accuracy basis is not verified. Do not treat its display
  alone as a calibration standard; use the water-column reference and preserve instrument limits.
- A controlled positive-pressure and vacuum fixture must avoid excessive stimulus and liquid entry.
  Do not connect the HVAC pump directly to the sensor.
- Zero capture depends on an actually atmospheric pressure path and stable measurements, neither
  of which firmware can prove from the button press. Keep confirmation explicit and rejection safe.
- USB CDC output previously affected responsiveness when its consumer was absent. Diagnostics must
  remain bounded/best-effort and must not be required for sampling or display behavior.
- The finished XIAO gauge, vehicle installation, threshold editing, and a full multi-point
  calibration workflow are outside M02; keep interfaces reusable without implementing those
  future features now.
