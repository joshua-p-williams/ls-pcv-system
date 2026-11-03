# Measurement adapters

The measurement code connects the physical ADS1115 to the hardware-independent pressure rules
in `gauge-core`. The ADC measures a voltage; the pressure converter interprets that voltage using
the provisional FTP-sensor family model. Keeping those steps separate makes each layer testable and
keeps the display from owning hardware behavior.

## Data path

```text
ADS1115 A0 voltage
  -> signed ADC conversion code
  -> microvolts using the selected PGA scale
  -> pressure conversion, with sensor-domain rejection
  -> MeasurementTracker updates current state, extrema, and trend
  -> immutable MeasurementSnapshot
```

`Ads1115Device` is a small register driver for the one required input, single-ended A0. It probes a
register to check that a device responds at the configured address, starts one conversion, polls
the conversion-ready bit, and reads the signed conversion register. The ADS1115 has no identity
register, so a response at `0x48` proves communication only; it does not authenticate the chip on
the shared-marking breakout board.

The config word `0xC183` is assembled from TI's ADS1115 configuration fields:

| Field | Bits/value | Meaning in this project |
|---|---|---|
| OS | `1` | Start a single conversion |
| MUX | `100` | Measure A0 relative to analog ground |
| PGA | `000` | Select `+/-6.144 V` conversion scale |
| MODE | `1` | Single-shot mode; convert on request |
| DR | `100` | Select `128 SPS` |
| COMP_QUE | `11` | Disable the ALERT/RDY comparator output |

The PGA value is a conversion scale, **not** permission to drive A0 above the ADS1115 supply. In
this design the sensor and ADC share nominal 5 V, and A0 must remain between the module's actual
ground and VDD. The selected `128 SPS` rate means a nominal conversion interval of about `7.8 ms`;
the source waits for readiness across successive polls rather than blocking a task for that time.
The 25 ms timeout and 10 ms I2C transaction timeout are bounded software safeguards and remain
subject to bench timing verification.

The conversion code is signed two's complement. `Ads1115Device` expands that representation
explicitly, then applies the exact `187.5 microvolt/code` scale using integer rounding. TI notes
that ADC offset can produce a negative single-ended code when the input is near 0 V. The source
treats exactly one negative code (`-1`, or `-187.5 µV` at this PGA setting) as the zero boundary,
while retaining the signed raw code for diagnostics. This matches the grounded-input bench
observation without turning a near-zero voltage into a pressure: zero volts remains outside the
provisional FTP sensor output domain. Codes of `-2` or lower remain ADC faults because they are
beyond this narrowly bounded zero-offset tolerance. The pressure sign comes later from the sensor's
atmospheric reference; voltage below that reference means positive crankcase pressure, and voltage
above it means vacuum.

## Bus and source ownership

`I2cRegisterBus` limits the ADC driver to checked 16-bit register operations. The N16R8
`ArduinoWireBus` adapts that contract to Arduino `Wire`, sets the W01 GPIO8/GPIO9 mapping and
100 kHz candidate, and applies a finite transaction timeout. Native tests substitute a fake bus to
exercise register order, configuration, readiness, sign extension, voltage scaling, NACK/timeout
handling, and bad conversion states without needing an ESP32 or ADS1115.

`Ads1115PressureSource::poll()` is a conversion-nonblocking state-machine step intended to run in
the acquisition task. It performs at most one bounded I2C transaction per call, retries
initialization at a bounded interval, and records each completed conversion into
`MeasurementTracker` before `snapshot()` returns. Arduino `Wire` transactions are synchronous;
the board adapter sets a 10 ms timeout so one bus operation has a finite bound. An I2C fault,
conversion timeout, below-ground code, or voltage outside the provisional pressure profile
suppresses the numeric pressure. No routine serial output or LVGL dependency exists in this layer.

## Phase 3 voltage-acquisition bench build

The PlatformIO environment `n16r8_adc` composes the N16R8 `Wire` adapter with
`Ads1115PressureSource`. A FreeRTOS acquisition task owns every ADS1115 transaction and publishes
immutable snapshots through a one-entry mailbox. The Arduino loop prints the latest raw code and
derived A0 voltage in microvolts over USB no more than four times per second. If monitor output is
slow or absent, older diagnostic snapshots may be skipped; the acquisition task continues to
process conversions. This is useful when display refresh is not part of the electrical test.

The log identifies ADC configuration, acquisition state, sequence number, acquisition time, raw
code, and calculated voltage. `voltage_read` means a new code and its calculated voltage were read;
it does not mean the electrical input is inside the FTP sensor's provisional pressure domain. The
voltage is derived from the ADS1115 code and selected gain; it is not an independent voltmeter
reading. Compare it with the multimeter connected to the same A0-side node. The firmware
intentionally omits pressure from its reports because a battery is an electrical test source, not a
calibrated pressure sensor. Keep the FTP signal disconnected for this phase.

Build and upload from `firmware/` with `pio run -e n16r8_adc` and
`pio run -e n16r8_adc -t upload`; open the 115200 baud monitor to read `M02_ADC` lines. The
`n16r8` live application also consumes this acquisition source; the M01 synthetic display remains
available as `n16r8_demo`. A successful build or a
plausible-looking log does not establish safe rails, physical ADS1115 identity, I2C voltage levels,
module pinout, or analog accuracy; those are recorded separately during bench verification.

The live integration is documented in the [firmware guide](../../README.md) and M02 feature tasks.
Host tests and a target build verify software behavior only; they do not demonstrate actual module
identity, analog voltage accuracy, sensor calibration, or physical fault detection.

## Reference

Register meanings and scaling follow the preserved [TI ADS1115 datasheet](../../../hardware/datasheets/ads1115-datasheet.pdf),
particularly the Configuration Register, conversion register, and programmable-gain tables. The
`+/-6.144 V` PGA range and `128 SPS` rate are the documented W01 starting choices. The pin mapping,
address selection, breakout pull-ups, and actual bus operation remain subject to the bring-up
checks in [W01 bench wiring](../../../hardware/wiring/bench/README.md).
