# Research: Live Sensor-to-Display Measurement Path

## Purpose and scope

This document records the facts and working recommendations for M02's approved
[feature](feature.md). It investigates the installed FTP sensor's published family references,
the ADS1115 measurement path, pressure conversion, bench references, zero storage, and current
firmware boundaries. It does not claim that the purchased replacement sensor or the bench
instruments have been calibrated or independently verified. Concrete module boundaries belong in
the architecture document; the implementation sequence and verification steps belong in the plan
and task documents.

## Current project configuration

- The firmware is PlatformIO with Arduino, currently pinned to `espressif32@8.0.0`. The N16R8
  environment currently includes LVGL and LovyanGFX; it has no ADS1115 driver or sensor acquisition
  implementation yet.
- M01's application core is hardware-independent C++. It stores display pressure as signed
  `PressureTenthsInH2O`, a tenth-inH2O integer. M01's `PressureClassifier` has the approved
  boundaries and exact inclusivity below, but `PressureBand`, `GaugeSession`, and the existing three
  page UI describe the synthetic demonstration. They do not yet represent measurement validity,
  sensor faults, or the fourth Settings page. M02 must introduce those meanings without presenting
  stale or invalid sensor data as a current pressure.
- M01's elapsed-time trend stores one sample per second in a 30-entry ring. The live acquisition
  rate and display refresh rate are separate design concerns; one-second trend history should not
  be confused with how often the ADC is read.
- The current Arduino loop services LVGL and the session. Synchronous display refresh can occupy
  the loop for hundreds of milliseconds; M01 already moved button polling to a separate FreeRTOS
  task for this reason. A live measurement and peak path that runs only when the UI loop is
  available could miss samples during display work. The architecture needs an acquisition timing
  and ownership boundary that preserves readings and extrema while keeping LVGL calls in its
  established owner context.
- W01 remains the connection authority. The selected analog path is FTP signal through the built
  `470 ohm` / `1 uF` RC filter to the purchased ADS1115 module's A0, with the sensor and ADC sharing
  nominal 5 V and the ADC's I2C translated to the ESP32-S3 3.3 V domain. The BOM and product
  references establish ADS1115 as the working part selection. W01's terminal map and configuration
  are design inputs; rail, bus, and conversion checks are routine bring-up, not prerequisites to
  choosing the software architecture.

## FTP sensor family references and conversion

### Published points

The unmarked purchased sensor listing claims compatibility with GM family number 16238399. The
2002 Impala 3.4 L service description for that family reports the following reference behavior:

| Differential pressure relative to atmosphere | Signal reference | What the source establishes |
| ---: | ---: | --- |
| `+4.5 inH2O` | `0.5 +/- 0.2 V` | One positive-pressure service reference point |
| `0 inH2O` | `1.3-1.7 V` | Expected output when tank pressure equals outside pressure |
| `-14 inH2O` | approximately `4.5 V` | One vacuum service reference point |

The service description says the sensor measures tank pressure relative to outside air. Its signal
voltage falls as pressure becomes more positive and rises as vacuum increases. These points are
asymmetric; the output is not centered at 2.5 V and the positive and negative ranges are not equal.
The service page is a family reference for an OEM application, not a datasheet for the purchased
aftermarket unit. It does not provide a complete transfer curve, accuracy specification, guaranteed
full-scale endpoints, or clipping/saturation behavior. The purchased listing's cross-reference is
the project's working identity, not proof of OEM manufacture.

Sources: [2002 Impala FTP sensor service description](https://charm.li/Chevrolet/2002/Impala%20V6-3.4L%20VIN%20E/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Fuel%20Tank%20Pressure%20Sensor/Description%20and%20Operation/) and the [sensor component record](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#published-reference-points-for-the-advertised-gm-family).

### Candidate family-based conversion

For a first software conversion while the actual unit is being characterized, the reference points
support a **piecewise-linear working approximation**, with the midpoint of the atmospheric range,
`1.5 V`, used as the nominal zero-signal reference. This is an interpolation between service
points, not a manufacturer-provided equation or a measured calibration:

```text
For signal voltage V <= Vzero:
    pressure_inH2O = (Vzero - V) * 4.5

For signal voltage V > Vzero:
    pressure_inH2O = -(V - Vzero) * (14 / 3)

Initial family reference: Vzero = 1.5 V
```

The positive-pressure branch follows the nominal points from `1.5 V, 0 inH2O` to
`0.5 V, +4.5 inH2O`, or about `+4.5 inH2O/V`. The vacuum branch follows `1.5 V, 0 inH2O` to
`4.5 V, -14 inH2O`, or about `-4.67 inH2O/V`. A captured atmospheric zero can replace `Vzero`
without changing either slope. A one-point adjustment cannot correct slope error or establish
linearity.

The pressure conversion must use the complete installed signal chain and measured pressure points
before the displayed value is treated as calibrated. Prefer fitting or interpolating the measured
points within the characterized domain; never extend the equation beyond it. The service anchors
can seed a provisional default when no user zero is stored, but the domain `-14 to +4.5 inH2O` is
not yet a verified operating range for the aftermarket unit. A point at an ADC rail is an
electrical condition, not proof of a particular pressure.

The service data's atmospheric window spans `0.4 V`. Applying the nominal vacuum-branch slope to
that width corresponds to about `1.9 inH2O`; the `+/-0.2 V` tolerance at the positive reference
point corresponds to about `+/-0.9 inH2O` under the nominal positive slope. These calculations
illustrate why the service references do not justify tenth-inH2O measurement accuracy. They are
not error bars for the purchased sensor: the service output windows are not stated as a guaranteed
accuracy specification. A display resolution of `0.1 inH2O` is useful formatting, not evidence of
`0.1 inH2O` accuracy.

### Initial status bands and exact boundaries

The project's initial live status guidance is documented in the
[sensor component record](../../../../hardware/components/fuel-tank-pressure-sensor/README.md#implications-for-gauge-limits):
lower danger `-12`, lower warning `-8`, upper warning `-3`, and upper danger `0 inH2O`. This
continues the M01 display boundaries, with the evidence and limitations now recorded for live
readings. The thresholds are configuration values, not settings-menu items in M02.

M01's current classifier applies strict crossings, giving these intervals:

| Pressure interval | Status |
| --- | --- |
| `p < -12 inH2O` | Danger |
| `-12 <= p < -8 inH2O` | Warning |
| `-8 <= p <= -3 inH2O` | Normal |
| `-3 < p <= 0 inH2O` | Warning |
| `p > 0 inH2O` | Danger |

Check sensor validity and the calibrated measurement domain **before** this status classification.
An out-of-range value is not a red-band pressure value. A failed I2C conversion, electrical
failure, known sensor fault, and a valid reading in a danger band have different meanings and need
separate data states. Exact representable rounding at threshold boundaries must be covered by
host tests when M02 defines the fixed-point pressure contract.

The current selected RC stage has a series resistor and a shunt capacitor, but no explicit
sensor-disconnect bias or diagnostic circuit. If the signal lead opens, A0 may float or retain a
charge and could temporarily produce a plausible-looking conversion. Firmware can identify a
failed I2C transaction, unavailable/stale conversion, an out-of-domain voltage, or a reading near a
known rail; those checks do **not** guarantee detection of every sensor or harness open/short.
The feature requirement to distinguish faults should be implemented only for observable fault
classes. Reliable wire-break detection would require a verified sensor failure signature or a
deliberate hardware diagnostic; neither is currently selected. Do not add diagnostic parts to W01
without a reviewed circuit change.

The thresholds provide initial direction and a useful gauge display. GM TechLink's
`-1 to -5 inH2O` hot-idle range, positive-pressure concern, and `-16 inH2O` excessive-vacuum
example apply to particular 1.4 L/1.5 L engines, not this modified 408. They are context, not
transferred engine specifications. See the [GM TechLink crankcase-pressure procedure](https://gm-techlink.com/wp-content/uploads/2017/04/GM_TechLink_04_Mid-February_2017.pdf).

## ADS1115 measurement constraints

TI's ADS1115 supports a `2.0-5.5 V` supply, a 16-bit conversion result, programmable full-scale
ranges through `+/-6.144 V`, and selectable data rates including `128 SPS`. At `+/-6.144 V`, the
nominal code step is `187.5 uV`. With the family slopes above, this maps to roughly
`0.00084-0.00088 inH2O` per ideal code near the two branches. That is far finer than the gauge's
`0.1 inH2O` display step; it demonstrates converter code resolution, not system accuracy. Sensor
variation, filter/loading, wiring, reference uncertainty, ADC noise, and calibration dominate the
real measurement error.

The ADS1115's PGA setting describes conversion scale; it does not authorize an input voltage above
the supply. TI specifies analog input pins from GND to VDD in normal operation. Its `+/-6.144 V`
range is appropriate for the selected 5 V design's nominal 0-5 V signal, but neither an input over
VDD nor an unpowered ADC input is allowed by that setting. Keep the ADS1115 and sensor on the
selected shared 5 V branch; verify the actual module rail and its behavior during bring-up. The
ADS1115 single-ended A0 mode measures a nonnegative voltage relative to ground; signed crankcase
pressure is calculated in software relative to the atmospheric sensor reference, not represented
by a negative ADC input voltage.

TI's rates are conversions per second, not a guarantee of equivalent fresh readings after software
overhead or I2C delays. At `128 SPS`, one conversion takes nominally `7.8 ms`. W01's `128 SPS`
setting is a bring-up candidate and matches the project's earlier `100-200 samples/s` acquisition
goal for a single channel. It should be tested with the display active and compared with the
sensor/tubing response and RC filter. The ADS1115 multiplexes its inputs; its maximum rate is not
available independently on all four channels. M02 uses only A0.

The selected RC stage is nominally `470 ohm` series and `1 uF` from OUT to ground. Its ideal time
constant is `0.47 ms` and one-pole cutoff is approximately `339 Hz`; actual response also depends
on sensor output impedance, module input, component tolerance, and effective capacitance. This
filter affects the analog peak response, unlike display smoothing. The complete chain's dynamic
response and peak capture must be measured; the nominal calculation alone does not establish a
maximum captured event duration. See the [RC filter record](../../../../hardware/components/rc-input-filter/README.md).

The BOM and purchased product reference select an ADS1115 module, so M02 uses the ADS1115 as its
working device. The supplied board image shows a shared `GY-ADS1115/ADS1015` PCB marking; that
silkscreen is ambiguous, but it does not outweigh the selected product listing or require a
separate identity investigation before design proceeds. ADS1015 is a distinct 12-bit device with
different output coding; if normal bring-up or voltage-comparison results conflict with ADS1115
behavior, revisit the assumption and adjust the driver/configuration before interpreting pressure.
The ADC component record and P03 remain the authority for the selected module mapping and
configuration candidates.

### Battery input check

The planned battery test isolates ADC voltage acquisition from sensor behavior. With the ADC
powered from its normal 5 V branch, connect only a measured battery source to the A0-side test
point and shared ground; disconnect the FTP signal during this check. Measure the voltage at the
ADC input node, not just at the battery, and compare repeated ADS readings to the multimeter.
Choose only sources whose measured voltage stays between the actual ADS1115 ground and supply
rails. Do not test with a source that can exceed VDD, connect an input to an unpowered ADC, or
interpret a battery comparison as pressure calibration. Record the battery type, its measured
voltage at the test node, ADS configuration and spread of readings.

A handheld multimeter comparison is a useful wiring/configuration check. Its own accuracy and
resolution limit the comparison; without a calibration certificate or known voltage reference,
this does not establish the ADC's absolute accuracy. The test should establish that code changes in
the correct direction and roughly track the measured voltage across several safe inputs.

## Pressure reference and safe stimulus

### U-tube manometer

A liquid-column manometer measures differential pressure through `DeltaP = rho * g * h`, where
`h` is the **vertical difference between the two liquid surfaces**. With water and an atmospheric
reference, the measured height difference is directly readable as a practical `inH2O` value; do
not use only one leg's movement. NIST's unit tables distinguish water-column units by reference
temperature; for this low-pressure prototype, record water temperature if it matters to the
desired comparison precision. A vertical scale, viewing the menisci at eye height, and repeated
approach from both pressure directions reduce avoidable reading error.

The published sensor-family points correspond to small pressures: `14 inH2O` is about `0.51 psi`
and `4.5 inH2O` is about `0.16 psi`. A vacuum pump/manifold that reads in `inHg` can apply far more
pressure difference than the sensor range. Do not connect the HVAC vacuum pump directly. Use a
manually controllable low-pressure source, a tee to the sensor and references, a way to isolate and
slowly bleed/equalize pressure, and a liquid catch/barrier so water cannot enter the sensor. The
exact practical fixture and stop range belong in the plan and must stay inside measured instrument
and sensor limits.

The U-tube is the best available low-pressure direct comparison for this project if its scale can
be read repeatably. The purchased digital manometer is useful as a second indication and for
convenience, but its nameplate and accuracy need identification before it is treated as a
calibration standard.

### Purchased digital manometer: limits of online identification

The purchase record identifies the Amazon listing as [ASIN B0F59W4P1H](https://www.amazon.com/dp/B0F59W4P1H/).
Its listing advertises `+/-36.80 kPa` (`+/-147.7 inH2O`) and a `0.3%` accuracy claim, but the
available listing text does not establish whether accuracy is percent of reading, full scale, or
another basis, nor does it identify a verifiable make/model. If `0.3%` were percent of the
advertised full scale, it would imply about `+/-0.44 inH2O` at full scale, a substantial uncertainty
near the project's `3-8 inH2O` target range. This is conditional arithmetic, not a verified
specification for the received unit.

An online search surfaced an AUTOOL-branded generic digital manometer manual with a different
published range (`+/-83.02 inH2O`) and `0.3% FSO`; its identity does not match the ASIN or its
advertised range, so it is **not** used as the purchased device's specification. When the meter is
physically available, record its label/model, actual units, resolution, zero behavior, response,
range and supplied manual. Until then, use the U-tube as the primary low-pressure reference and
the digital unit only as a cross-check whose limits are recorded.

For pressure/vacuum correlation, record points in both directions and repeat atmospheric zero.
Preserve raw data rather than writing a fitted curve over the source observations. A useful record
includes applied pressure, U-tube level difference, digital-meter reading and mode, sensor supply,
sensor output voltage, voltage at ADS A0, ADC chip/configuration, raw conversion code, converted
pressure, direction of approach, and notes about stability or oscillation. The actual test matrix
and acceptable residuals are plan/architecture decisions informed by the measured instruments.

## Atmospheric zero adjustment and storage

The family reference describes an output of `1.3-1.7 V` when tank pressure equals outside
atmospheric pressure. This supports interpreting the sensor as differential to ambient, rather
than an absolute barometer. Ordinary weather changes should affect both the crankcase reference
and outside-air reference in common mode; a user zero is for installed-chain offset, not a routine
barometric correction. This conclusion assumes the replacement sensor preserves the advertised
family behavior and installation reference.

The approved zero workflow requires an explicit user action and confirmation, with the engine off
and cold, oil cap removed to expose the crankcase to atmosphere, and capture only after the reading
settles. The device cannot confirm engine state, port routing, or atmospheric equalization. Never
zero automatically while the engine is running.

Research supports ESP32 Arduino's `Preferences` as the appropriate small-settings facility: it
stores key/value data in ESP32 NVS, survives resets and power loss, is available on ESP32 variants,
and is documented as the replacement for Arduino EEPROM. The API supports small numeric values and
reports whether a write succeeds. Store only after the confirmed action, check persistence
operations, and fall back to the family conversion if the stored zero is absent or invalid. The
zero belongs to the specific sensor/conditioning/ADC chain; a stored N16R8 bench value should not
be presumed transferable to a different finished-gauge sensor chain.

For a one-point zero, store the measured atmospheric signal reference and use it to offset the
family or measured transfer relationship; do not store a new slope from that one point. A voltage
reference is independent of the ADS gain-code scale, while a raw ADC code depends on gain and ADC
identity. The final storage representation, record validity/versioning, stability statistic and
plausibility bounds should be chosen in architecture. A short averaged sample window with a
stability check is preferable to capturing one noisy conversion; the exact duration and rejection
threshold must be set from measured bench noise, not guessed here.

If storage fails, the interface must not report success. On boot, distinguish a valid saved zero
from the family default. Reset-to-default behavior should remove/disable the saved offset without
changing the firmware threshold constants. Preferences/NVS behavior is documented by [Espressif's
Arduino-ESP32 API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html).

## Firmware library and test findings

No ADC library is present in the current PlatformIO configuration. Adafruit's
[ADS1X15 Arduino library](https://github.com/adafruit/Adafruit_ADS1X15) supports both ADS1015 and
ADS1115 devices and uses the BSD license; its metadata lists `Adafruit BusIO` as a dependency. It is
a reasonable library candidate if the received IC and required nonblocking conversion pattern are
supported. This research does not add a dependency or choose the final driver. A small direct
register implementation is an alternative, but would make M02 responsible for more low-level I2C
and conversion details; architecture should compare the real APIs, maintenance, blocking behavior,
and test seam before selecting.

Pressure conversion, zero correction, status classification and validity decisions can be
implemented as pure C++ logic, building on `firmware/lib/gauge-core` and its native Unity tests.
The Arduino adapter should own physical I2C/ADC interaction; UI rendering should receive a stable
measurement snapshot and never perform sensor math. The active source remains explicit at build
composition: a normal live-sensor environment and a separate synthetic demo environment. The M01
demo source and its gesture mappings remain unchanged in meaning.

At minimum, automated tests should exercise:

- The nominal piecewise family conversion at atmospheric, positive, and vacuum points, including
  signal polarity, branch selection, and conversion to the display's tenth-inH2O unit.
- User zero offset application without slope change, rejection of invalid/unstable zero samples,
  stored-value validation and fallback to the default when storage is missing or invalid.
- Exact pressure-band boundaries, including equality behavior inherited from M01, plus the rule
  that invalid/out-of-domain measurements never enter pressure classification or extrema.
- ADC error, stale/unavailable sample, sensor voltage outside its characterized domain, and
  detected near-rail/saturation states as distinct outcomes where the hardware makes them
  observable.
- Live trend cadence and extrema behavior with repeated samples and invalid intervals; UI stalls
  must not silently fabricate current values or erase a captured peak.

Automated tests cannot establish sensor calibration, assembled ADC/I2C electrical behavior, analog
protection, manometer accuracy, or physical display behavior. Those need recorded bench checks.

## Recommendations for architecture and planning

1. Keep pressure math and validity in a hardware-independent model. Use the published family points
   as explicit provisional defaults, then apply a stored zero reference and measured calibration
   within a separately defined valid domain. Keep ADC errors, sensor faults, out-of-domain pressure,
   and valid warning/danger pressure distinguishable.
2. Use the purchased ADS1115 selection, W01 A0 mapping, and documented address/rate candidates as
   the implementation basis. During normal bring-up, confirm safe rail levels, communication, and
   voltage tracking before connecting the sensor or interpreting pressure. Investigate unexpected
   behavior rather than treating the listing-based selection as disproven in advance.
3. Start at the selected `128 SPS` single-channel candidate, verify fresh-conversion timing and
   peak response, and keep acquisition independent of the display's synchronous work. Maintain the
   existing one-second trend as a separate display-history cadence.
4. Use the U-tube as the low-pressure primary reference and the purchased digital meter as a
   corroborating tool until its exact specification is matched. Use a low-pressure controllable
   stimulus and a water barrier; exclude direct HVAC-pump connection.
5. Treat atmospheric zero as one-point offset trim only. Save after explicit confirmation, verify
   the saved value, and retain a known fallback conversion when it is unavailable.

These are research recommendations for the architecture and plan review, not implementation
authorization.

## Questions requiring physical evidence

The purchase listing, BOM, and supplied module references are the working basis for component
selection. The remaining physical checks below are ordinary commissioning or measurement work;
they are not prerequisites to the M02 software architecture:

- Does the assembled ADC/I2C path power, communicate, and track safe known voltages as expected
  under W01 wiring and the selected configuration? If it does not, inspect the affected connection
  or revisit the working module assumption.
- What measured voltage does the assembled sensor/RC/ADC chain report at atmospheric equalization,
  and what repeatable pressure/voltage points does the actual sensor produce in both directions?
- What make/model, selectable range, resolution, accuracy basis and zero behavior apply to the
  purchased digital manometer? Does its reading agree with the water-column reference near the
  pressure range of interest?
- What pressure fixture can apply and bleed gentle positive and negative pressure with a visible
  reference while preventing liquid entry and staying within the verified test limits?
- What are the stock R1/C1 values, actual assembled filter response, and measured sample/peak timing
  under display activity?

No further product preference is needed to proceed to architecture drafting. If physical evidence
later contradicts an accepted listing or family assumption, record the discrepancy and revise the
affected design before using its pressure values.

## References

| Reference | Research use |
| --- | --- |
| [TI ADS111x datasheet, SBAS444E, revised December 2024](https://www.ti.com/lit/ds/symlink/ads1115.pdf) | Supply and absolute input limits, PGA/LSB sizes, data rates, conversion timing, input multiplexing, and ADC error/noise information. A local copy is preserved at `hardware/datasheets/ads1115-datasheet.pdf`. |
| [GM 2002 Impala FTP sensor service description](https://charm.li/Chevrolet/2002/Impala%20V6-3.4L%20VIN%20E/Repair%20and%20Diagnosis/Powertrain%20Management/Computers%20and%20Control%20Systems/Fuel%20Tank%20Pressure%20Sensor/Description%20and%20Operation/) | Family reference voltages and pressure points; not a specification for the purchased aftermarket unit. |
| [GM TechLink, “Checking Crankcase Pressures,” February 2017](https://gm-techlink.com/wp-content/uploads/2017/04/GM_TechLink_04_Mid-February_2017.pdf) | Contextual examples of pressure concerns on the listed 1.4 L/1.5 L engines; not limits for this 408. |
| [Espressif Arduino-ESP32 Preferences API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html) | NVS-backed storage behavior, supported value types, and write success reporting. |
| [Adafruit ADS1X15 Arduino library](https://github.com/adafruit/Adafruit_ADS1X15) and [library metadata](https://github.com/adafruit/Adafruit_ADS1X15/blob/master/library.properties) | Candidate Arduino driver for ADS1015/ADS1115; BSD license and BusIO dependency. |
| [NIST Guide to the SI, Appendix B.9](https://www.nist.gov/pml/special-publication-811/nist-guide-si-appendix-b-conversion-factors/nist-guide-si-appendix-b9) and [NIST pressure measurement note](https://www.nist.gov/system/files/documents/calibrations/pmc-2.pdf) | Water-column pressure relation, pressure-unit reference temperature, and manometer level-difference interpretation. |
| [Purchased digital manometer listing, ASIN B0F59W4P1H](https://www.amazon.com/dp/B0F59W4P1H/) | User-supplied product reference; currently not sufficient to identify a verified model or accuracy basis. |

Datasheet and official API facts above were reviewed on 2026-11-01. The project component pages,
preserved datasheet and W01 remain the durable hardware/interface records; this feature research
summarizes only the facts needed to design M02.
