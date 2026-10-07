# XIAO ESP32-S3 interface

P02 documents the standard Seeed Studio XIAO ESP32-S3 intended for the finished gauge and a matching bench build. Pin functions below use manufacturer references; the received boards and their revisions still need physical verification. This is an interface reference, with project wiring assignments kept separate.

## Scope and identity

The [purchase record](../components/gauge-electronics/README.md) identifies the standard XIAO ESP32-S3, ASIN B0DJ6NQFKX, supplied as three boards. The [product image](../../media/reference/gauge-electronics/xiao-esp32s3-pack.jpg) shows base boards and antennas. It does not establish the received revision. Sense expansion hardware and the Plus variant are outside this interface's scope.

Seeed specifies 8 MB flash and 8 MB PSRAM for the standard board. Its [series guide](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) covers multiple variants; use the standard-board sections and confirm the actual markings before applying a variant-specific schematic.

## Orientation and edge pins

View the component side with USB-C at the top and the antenna connector toward the bottom, as in Seeed's [front pinout image](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/img/XIAO_ESP32-S3_front_pinout.png). Left and right are defined from that view. Count rows downward from USB-C. Row numbers are documentation coordinates, not chip pin numbers. A bottom-side view reverses left/right.

| Row | Left label | GPIO / function | Right label | GPIO / function |
| --- | --- | --- | --- | --- |
| 1 | D0 | GPIO1 | 5V / VBUS | USB power rail |
| 2 | D1 | GPIO2 | GND | Ground |
| 3 | D2 | GPIO3 | 3V3 | Regulated output |
| 4 | D3 | GPIO4 | D10 | GPIO9 / MOSI |
| 5 | D4 | GPIO5 / SDA | D9 | GPIO8 / MISO |
| 6 | D5 | GPIO6 / SCL | D8 | GPIO7 / SCK |
| 7 | D6 | GPIO43 / TX | D7 | GPIO44 / RX |

Evidence: **Manufacturer Spec**, transcribed from the linked front pinout and cross-checked against the schematic's edge-interface block. These mappings describe board aliases, not chosen gauge connections. In particular, `D4` means GPIO5, not GPIO4. Do not copy the [N16R8 bench board's](esp32-s3-devkit.md) header coordinates into a XIAO wiring plan.

## Signal interfaces and project allocation

The bus labels identify the manufacturer's conventional assignments. Firmware and harness configuration must agree on the actual choices. See Seeed's [pin-multiplexing guide](https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/) for I2C, SPI and UART usage.

| Interface | Board alias / GPIO | Direction at controller | Project use |
| --- | --- | --- | --- |
| I2C data | D4 / GPIO5 | Bidirectional | Candidate ADS1115 SDA; assignment pending |
| I2C clock | D5 / GPIO6 | Output with bus input sensing | Candidate ADS1115 SCL; check 3.3 V pull-ups |
| SPI clock | D8 / GPIO7 | Output | Candidate GC9A01 clock |
| SPI transmit | D10 / GPIO9 | Output | Candidate GC9A01 data/MOSI |
| SPI receive | D9 / GPIO8 | Input when used as MISO | No readback signal established for the selected display; availability for another role requires a pin plan |
| UART transmit | D6 / GPIO43 | Output | Keep console/debug requirements explicit before reuse |
| UART receive | D7 / GPIO44 | Input | Keep console/debug requirements explicit before reuse |
| Display CS, DC, reset | TBD | Outputs | Not assigned by the SPI clock/data labels |
| Zero / peak-reset controls | TBD | Inputs | Button count and pin selection pending |
| ADS1115 ready indication | TBD, if used | Input | Optional; acquisition method unresolved |

GPIO interfaces use the project's 3.3 V logic domain. The FTP analog signal goes through conditioning and the ADS1115, not directly to a XIAO ADC pin. See the [measurement chain](../architecture/measurement-system.md).

A preliminary pin budget is useful before the harness: two I2C lines plus five display lines (clock, data, CS, DC, reset) consume seven of the eleven edge GPIOs. Two separate buttons would bring that to nine. Keeping D2 unused and retaining both UART pins would leave only eight. This calculation assumes dedicated signals; control count, reset strategy, optional ADC-ready and debug choices need to be resolved together. It does not select pins or require extra hardware.

## Power interfaces

| Label / interface | Function | Project decision and remaining check |
| --- | --- | --- |
| USB-C | Programming/data and 5 V power | Identify the intended bench mode and power interaction |
| 5V / VBUS | USB-linked rail; external-input use has conditions | Resolve isolation/backfeed design before using the converter with a USB host |
| 3V3 | Regulated supply output | Selected source for ADS1115 and compatible peripherals; establish actual current margin |
| GND | Supply and signal return | Verify physical return arrangement with the harness |
| Battery pads | Battery/charging interface | Not selected for this gauge; do not use as a 5 V input |

Seeed's [power-pin guidance](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/#power-pins) specifies a series diode for external input to the 5V pin, oriented from the external source toward that pin. That instruction alone does not establish a complete simultaneous USB/external-power solution for this project.

The wiki states 700 mA for 3V3. The [linked schematic](https://files.seeedstudio.com/wiki/SeeedStudio-XIAO-ESP32S3/new-res/202003751_XIAO%20ESP32S3_v1.4_SCH_260226.pdf.pdf), sheet 4, labels the regulator output `Imax=600mA`. These figures conflict and neither establishes measured peripheral headroom for the purchased boards. Check revision, board consumption, thermal conditions and intended loads before assigning a budget. The converter's 3 A rating does not resolve this difference.

The schematic also connects the edge VBUS pin and USB VBUS to the same net; its downstream regulator diode should not be assumed to isolate an external supply at that header from the USB host. Confirm applicability to the actual revision. The [power architecture](../architecture/power-system.md) retains the unresolved combined-power mode, while the FTP sensor remains planned for 5 V and display supply compatibility remains open.

## Restrictions and onboard functions

- **D2 / GPIO3:** a strapping pin; avoid it in the initial gauge allocation until reset-time loading is evaluated. GPIO0, GPIO45 and GPIO46 also have strapping roles. See [Espressif's GPIO reference](https://docs.espressif.com/projects/esp-idf/en/v5.3.3/esp32s3/api-reference/peripherals/gpio.html).
- **Native USB:** GPIO19/20 serve USB and are not edge D-pin aliases. Preserve them for programming/debug use; the schematic shows the USB connection.
- **BOOT and reset:** the manufacturer pinout identifies GPIO0 and CHIP_PU respectively. These controls are not additional unallocated edge GPIOs.
- **User LED:** GPIO21 according to the manufacturer pinout; do not reuse another board's LED definition. The charge LED is a separate indicator.
- **Memory and extra contacts:** the edge map does not expose GPIO35-37. Do not count expansion contacts, test pads or D11/D12 as extra standard edge pins. Their accessibility and function depend on board/expansion details, outside this initial harness scope.

## Reference identity and verification

The schematic linked under Seeed's standard-board resources has inconsistent identifiers: the URL names v1.4, the title block says **XIAO ESP32-S3-Sense, V1.3, 2026-02-10**, and the root filename references V1.5. Its four-page document was visually inspected, including the edge mapping, USB and regulator paths. This is reference evidence, not a determination of the purchased PCB revision. Retain these identifiers when comparing a physical board or a replacement reference.

Before final wiring:

1. Compare the received board's front/back markings and edge labels with this map; identify the standard-board revision and matching schematic.
2. Confirm ground, 5V/VBUS and 3V3 paths, then measure rails in the chosen power mode before attaching peripherals.
3. Reconcile regulator documentation and measure the complete load budget, including controller activity and display load.
4. Select GPIOs after resolving UART/debug needs, button count, display controls and optional ADC-ready signaling.
5. Check the chosen firmware board definition's D aliases against explicit GPIOs, then record the mapping in the harness and [firmware configuration](../../firmware/README.md).

No physical continuity, power or functional tests are recorded yet. Wiring destinations belong in the [harness documentation](../wiring/README.md); procurement remains in the [BOM](../bom/parts.md).
