# M02 — Sensor-to-Display Measurement Path Brainstorm

This note captures the working discussion for the next firmware milestone. It is a brainstorming
record, not an approved feature specification, implementation plan, or authorization to begin
firmware changes. M02's scope and acceptance criteria will be defined in the review documents.

## Desired outcome

Replace M01's synthetic pressure source with a real measurement path from the fuel-tank-pressure
(FTP) sensor, through the ADS1115 analog-to-digital converter (ADC), into the existing gauge UI.
Develop and verify the path incrementally so voltage acquisition, pressure conversion, and display
behavior can be understood and tested separately.

The intended milestone is bench-focused. It carries the project's initial, research-informed live
status bands into the real-sensor display, but does not claim they are validated engine-damage
limits, establish the engine's operating baseline, or authorize vehicle testing. Published
sensor-family behavior remains a working reference until measurements characterize the installed
sensor.

## Working test sequence

1. **Verify the ADC with known voltages.** Use measured batteries below 5 V as simple voltage
   sources. Record each terminal voltage with a multimeter before connecting it to the ADC input;
   share the bench circuit's ground and keep the ADS1115 input within its supply rails. Compare
   multimeter readings with the ADS1115 results to check the electrical connection, I²C
   communication, ADC configuration, and voltage conversion independently of the FTP sensor.
   Battery testing is an electrical-input check, not a pressure calibration.
2. **Build a water-column U-tube manometer.** Use clear tubing, a vertical scale, and colored water
   to make the pressure difference visible and provide an inH2O reference. Keep the liquid isolated
   from the sensor and electronics with appropriate tubing arrangement and a catch/isolating
   measure. Read both water levels at eye height; their vertical difference is the differential
   pressure.
3. **Use the purchased digital differential manometer as a second reference.** The purchased
   Amazon listing is [ASIN B0F59W4P1H](https://www.amazon.com/dp/B0F59W4P1H/). The listing title
   advertises a ±36.80 kPa / ±147.7 inH2O range and claims 0.3% accuracy. The make/model and
   accuracy basis have not yet been independently verified. Its broad range may limit useful
   resolution near this sensor's expected pressures; compare it with the water column rather than
   treating its display resolution or listing claim as certified accuracy. Confirm its actual unit
   selection and zero behavior when it arrives.
4. **Apply controlled pressure and vacuum to the FTP sensor.** Start with gentle, manually
   controlled pressure or vacuum, monitor it with both references, and record the reference value,
   sensor output voltage, ADC result, and displayed pressure. Increase the test range gradually.
   Prevent water from reaching the sensor. Do not connect the HVAC vacuum pump directly without a
   suitable low-pressure control and isolation arrangement.
5. **Integrate the measured source into the existing gauge.** Keep the sensor/ADC measurement
   path separate from the display driver and preserve clear invalid, saturated, and unavailable
   reading behavior. Reuse M01's display screens where appropriate; review actual behavior before
   defining any production warnings or alarms.

## Current working references and constraints

- The selected bench setup uses a 5 V FTP sensor and ADS1115, with the ADS1115's I²C translated to
  the ESP32-S3's 3.3 V logic. The ADS1115 input must remain between its ground and supply rails;
  its programmable gain setting does not make an input above the supply safe.
- Published information for the advertised GM sensor family gives approximate reference points
  near +4.5 inH2O and −14 inH2O. These are family references, not measured limits or calibration
  results for the installed replacement sensor. See the [sensor component record](../../../../hardware/components/fuel-tank-pressure-sensor/README.md).
- The project's pressure convention is relative to atmosphere: negative is vacuum and positive is
  pressure. Report readings in `inH2O`.
- The water-column U-tube is a direct physical reference and a useful educational demonstration.
  The purchased digital manometer provides faster readings and an independent cross-check. Neither
  instrument is assumed to be calibrated or certified for this project until its scale, range,
  and uncertainty are understood.
- A reading that reaches the sensor's verified electrical or pressure limit must not be presented
  as a valid extrapolated measurement. Define the handling of clipping, sensor faults, and
  out-of-range conditions during research and feature design.

## Pressure-source selection and gestures

Keep the M01 synthetic pressure source available after the live sensor path is added. Select the
source at build time through explicit PlatformIO environments rather than requiring maintainers to
edit a source-code `#define` before each upload. The intended arrangement is a normal N16R8 build
using the ADS1115/FTP source and a separate N16R8 demo build using the synthetic source; exact
environment names and composition belong in architecture and plan review.

Both sources should implement the same small pressure-reading contract so the gauge and its pages
do not need separate display logic for synthetic and physical readings. Keep the source selection
in one composition point, test both implementations where practical, and document the selection
clearly in code and build instructions. Comments should explain why the synthetic source remains,
how each build selects it, and what diagnostics identify the active source; avoid scattered
conditional compilation that obscures runtime behavior.

In live-sensor mode, a long press navigates among the gauge, trend, min/max, and Settings pages.
On the measurement pages, single and double clicks do not modify or synthesize pressure and have no
assigned action. On the Settings page, a single click advances through the available options and a
double click selects or confirms the highlighted option; the initial option is `Set zero`. In the
demo build, retain M01's temporary click and double-click behavior for exercising synthetic values.
These M01/M02 gestures do not define later production alarm acknowledgement or broader settings
behavior.

## Atmospheric zero-reference adjustment

Include a user-initiated zero-reference adjustment in M02 for initial installation and after
relevant maintenance. Although it adds a small settings interaction and persistent state, it is
part of the sensor-to-display measurement path: it corrects the installed chain's pressure offset
before the gauge presents a pressure value. The intended adjustment does not recalibrate the
pressure/voltage slope, expand the sensor's range, or replace bench characterization against known
pressure references. Keep it independently testable from voltage acquisition and the default
sensor-family conversion.

The advertised GM sensor family measures crankcase pressure relative to outside atmospheric
pressure. Its published family reference is approximately 1.3–1.7 V when the sensed pressure equals
outside pressure. Assuming the installed replacement follows that behavior, ordinary weather-driven
barometric changes should be common to both sides of the differential measurement and should not
require repeated user zeroing. A zero trim is instead for the installed sensor's offset and should
only be captured when the crankcase and sensor pressure port are known to be at atmosphere.

The proposed interaction is a fourth Settings page reached by long-press page navigation. The first
option is `Set zero` (or another clearly scoped label). A double click opens an instruction and
confirmation screen; a single click changes the selected option if more are added later, and a
double click confirms the action. The prompt should require the engine to be off and cold, with the
oil cap removed so the crankcase is exposed to atmosphere; capture only after the reading has
settled. The device cannot independently verify engine state or a clear pressure path, so the
procedure and confirmation are user-controlled. Never perform zero capture while the engine is
running.

If no valid user zero is stored, use the documented family-based default conversion. If capture is
stable and accepted, save the zero adjustment to ESP32 non-volatile storage (Arduino Preferences /
NVS, rather than treating the ESP32 as if it had physical EEPROM). Save only after explicit user
confirmation, not continuously or on every boot. On startup, load a valid stored trim or fall back
to defaults. The UI should report whether zero was stored or why capture was rejected, such as an
unstable reading. Reset-to-default behavior and the exact stability/plausibility criteria remain
for research and architecture.

Use a user-facing label such as `Set zero` or `Zero reference`, not an unqualified `Calibrate`,
because a single atmospheric point adjusts offset only. Multi-point characterization/calibration
against the water column and digital manometer remains a separate bench activity.

## Live pressure status bands

The real-sensor gauge should retain green/yellow/red status bands. Lack of laboratory-grade engine
testing is not a reason to omit useful guidance. Carry M01's four established gauge boundaries into
live mode as the initial, research-informed defaults. Keep them centralized as firmware
configuration values so later evidence can refine them; editing warning/danger bands through the
Settings UI is outside the current M02 direction. Present them as the project's best current
operating guidance, not as certified engine limits or a guarantee against damage.

Use these established defaults, in increasing pressure order. The shared rationale and evidence
record is maintained in the [FTP sensor component documentation](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#implications-for-gauge-limits); this brainstorm records their intended firmware use:

| Boundary | Default | Interpretation |
| --- | ---: | --- |
| Lower danger | `-12 inH2O` | Approaching the sensor family's lower measurement boundary |
| Lower warning | `-8 inH2O` | Lower edge of the provisional idle/light-cruise target |
| Upper warning | `-3 inH2O` | Upper edge of the provisional idle/light-cruise target |
| Upper danger | `0 inH2O` | Positive crankcase pressure begins |

This gives a green target band from `-8 to -3 inH2O`, yellow warning bands from `-12 to -8` and
`-3 to 0 inH2O`, and red bands below `-12` or above `0 inH2O`. Published GM examples support
these as reasonable initial attention bands while also showing why they are not universal engine
limits: one TechLink procedure gives `-1 to -5 inH2O` at hot idle for particular 1.4 L/1.5 L
engines, calls positive pressure above `0 inH2O` excessive in those applications, and flags
pressure below `-16 inH2O` as excessively negative. Those values belong to different engines; use
them as contextual support for the direction of concern, not as specifications for this modified
408. Other GM applications have different ranges. See the
[GM TechLink crankcase-pressure procedure](https://gm-techlink.com/wp-content/uploads/2017/04/GM_TechLink_04_Mid-February_2017.pdf).

Keep the live status thresholds separate from the sensor's measurement limits. The family-level
sensor reference approaches its vacuum limit near `-14 inH2O`; choose visual bands that do not
depend on reporting extrapolated pressure below the verified sensor span. Out-of-range and sensor
fault indications remain distinct from ordinary warning/danger classification. The `-12 inH2O`
lower-danger boundary leaves margin before the provisional `-14 inH2O` sensor limit; readings
beyond the verified sensor span must be reported as out of range rather than extrapolated.

## Open design topics for the review documents

- Exact battery test procedure, safe input connection, selected test voltages, and ADC error
  acceptance criteria.
- The digital manometer's actual make/model, selectable range, resolution, accuracy basis, and
  suitability as a comparison reference.
- The default sensor-family transfer relationship, what the M02 reference tests can establish,
  and how the in-vehicle zero trim is kept distinct from multi-point bench calibration.
- U-tube dimensions, scale/readout method, fluid containment, and how positive and negative
  pressure will be generated safely and repeatably.
- Sensor connector pin mapping and the source-backed working assumptions already recorded in the
  hardware component documentation.
- Sensor voltage-to-pressure conversion, calibration method, uncertainty, and invalid/saturated
  reading behavior.
- Firmware interfaces for physical ADC readings and the retained synthetic test source; test
  coverage and how each source is selected and diagnosed.
- The research and architecture documents should determine how pressure samples are accepted,
  converted, filtered for display, and retained for fast extrema versus the one-second trend
  history. Keep peak capture responsive if live display smoothing is introduced.
- Exact Settings/zero-reference interaction, Preferences/NVS record validation, capture rejection,
  reset-to-default behavior, and user feedback for stored/default/rejected state.
- Which stages can be completed with the existing protoboard and which require any wiring changes.

## Boundaries

M02 should establish the bench measurement path and its evidence. It should not claim that the
sensor is calibrated merely because its output changes, present the initial live status bands as
validated engine-damage limits, automatically zero with the engine running, or proceed to vehicle
baseline testing without a separately defined and reviewed test plan.
