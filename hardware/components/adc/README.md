# ADS1115 acquisition modules

Selected external ADC: **ADS1115 modules**, [Amazon B0BXDLZLZS](https://www.amazon.com/dp/B0BXDLZLZS). Supplied notes confirm **three modules purchased for USD 5.98 total**. Planned allocation is one bench module, one finished-gauge module, and one spare/expansion module. Receipt, installation, chip identity, and operation remain unconfirmed.

The intended measurement path is FTP sensor, input scaling/protection/filtering, ADS1115, then I2C to ESP32-S3. The controller handles calibrated pressure, peak tracking, and display rendering. An external ADC keeps the acquisition design consistent between the N16R8 bench board and XIAO controller; changing the actual ADC or analog circuit still calls for calibration review.

See [A01: overall gauge architecture](../../architecture/gauge-system.md) for the ADC between analog conditioning and the controller, with signal and power boundaries distinguished.

See [A03: pressure measurement signal chain](../../architecture/measurement-system.md) for how conversion counts become calibrated pressure, validity state, peaks and display values.

See [P03: ADS1115 module interface](../../pinouts/ads1115.md) for header orientation, terminal functions, address options and electrical verification before wiring.

## Device specification reference

The exact reference revision is stored as the [local ADS111x datasheet](../../datasheets/ads1115-datasheet.pdf). See the [datasheet index](../../datasheets/README.md) for document identity, provenance, and checksum; retain the official TI links below for checking updates.

TI specifies the ADS1115 as a 16-bit delta-sigma ADC with a multiplexer for four single-ended or two differential measurements, programmable gain, internal reference/oscillator, and four selectable I2C addresses. Supply range is 2.0-5.5 V; the IC temperature range is -40 to +125°C. These IC specifications do not qualify the purchased breakout assembly. Reference: [TI ADS111x datasheet, SBAS444E, December 2024](https://www.ti.com/lit/ds/symlink/ads1115.pdf), consulted 2026-10-06.

Nominal output rates are **8, 16, 32, 64, 128, 250, 475, and 860 SPS**. The PGA range does not permit analog inputs beyond the supply rails in normal operation. Inputs are multiplexed, so the maximum conversion rate is not available independently on every channel at once. See sections 5.3, 7.3, and 8 of the [TI datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf).

## Supplied board references

![Vendor ADC front view](../../../media/reference/adc/adc-front.jpg)

![Vendor ADC back view](../../../media/reference/adc/adc-back.jpg)

The front silkscreen says `GY-ADS1115/ADS1015`, which does not establish which device is populated. Verify the received IC and behavior before relying on ADS1115 specifications. The rear image labels `V`, `G`, `SCL`, `SDA`, `ADDR`, `ALERT`, and `A0` through `A3`. These vendor views do not establish the actual board schematic, pull-up supply, address configuration, or ESP32 GPIO assignments.

## Preliminary electrical plan

- Power the ADC from the selected ESP32-S3 board's `3V3` output, subject to actual module compatibility and board current-budget verification. Check I2C pull-ups and bus voltage before connection.
- Use A0 for crankcase pressure initially, with A1-A3 available for future work. This is a proposed channel allocation, not implemented wiring.
- Scale the planned 5 V FTP signal before a 3.3 V-powered ADC. The supplied example places **12 kÎ© from signal to ADC node and 20 kÎ© from node to ground**: `20 / (12 + 20) = 0.625`, giving 3.125 V for an ideal 5.0 V input.
- Evaluate approximately **0.1 µF at the ADC input** as an initial filtering concept, plus appropriate supply decoupling and protection. Values are not finalized or validated.

The divider calculation checks only nominal scaling. Final design must account for sensor range/faults, resistor and supply tolerances, loading, acquisition settling, power sequencing, and protection. Select gain, conversion mode, address, wiring, and filter values together. Do not treat the divider example as a completed schematic.

## Sampling, peaks, and calibration

The notes propose approximately 100-200 samples/sec for acquisition and 20-30 display updates/sec. These are goals, not measured performance or final configuration. Choose a supported conversion rate (128 SPS lies within the proposed acquisition range), then measure effective throughput and peak response. Polling faster does not create new conversions. Channel expansion and display work must not silently reduce the pressure sample rate.

Keep peak capture separate from display smoothing, and evaluate the entire sensor, analog filter, ADC, and software response before claiming short spikes are captured. Resolution alone does not establish pressure accuracy or noise performance.

Calibrate the complete installed sensor/divider/ADC chain against a known pressure reference. Preserve raw counts, gain/rate/channel settings, module identity, analog component values, and calibration revision. Zero, slope, noise, linearity, and repeatability require measurement; a new module is not automatically interchangeable without rechecking calibration.

Future options include additional conditioned sensors, multiple displays, or additional addressed ADC modules. The notes prefer obtaining MAP digitally from Holley rather than using an analog channel; no Holley interface is implemented or verified. These options do not expand the initial one-pressure-channel scope.

## Import record - 2026-10-06

Imported project notes and two third-party product images from batch `ADC`. Public images are in `media/reference/adc/`; purchase details and selections are reflected in the [BOM](../../bom/parts.md), [gauge plan](../../components/gauge-electronics/README.md), and [firmware plan](../../../firmware/README.md).

Reviewed visible images for personal identifiers; none were observed. Removed embedded JPEG application/comment metadata without recompression, verifying unchanged orientation, dimensions, and decoded pixels. Preserved product labels; inbox originals remain unchanged. Product URL uses the canonical ASIN path.

Checked nominal divider arithmetic and TI's documented rates/input constraints. No physical inspection, module authentication, wiring, calibration, electrical testing, or firmware build was performed. The [test plan](../../../docs/testing/test-plan.md) remains the basis for physical validation.
