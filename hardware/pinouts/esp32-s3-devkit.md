# ESP32-S3 N16R8 bench-board interface

P01 documents the bench controller used by the [gauge architecture](../architecture/gauge-system.md). The header map below is transcribed from the supplied product image. Electrical functions are qualified against Espressif references where applicable; the received board, PCB revision and wiring have not been verified. The purchased board has not yet arrived; all supplied images are online references. [W01](../wiring/bench/README.md#proposed-signal-allocation) proposes project GPIO assignments, subject to actual-board verification.

## Board identity and evidence

| Item | Available evidence | Remaining check |
| --- | --- | --- |
| Purchase identifier | Amazon ASIN B0D93DLB6Q; see [component record](../components/esp32-s3-dev-board/README.md) | Identify actual PCB manufacturer and revision |
| Module marking in product image | `ESP32-S3-WROOM-1`, `N16R8` | Compare with the received module |
| Memory configuration | Espressif identifies WROOM-1-N16R8 as 16 MB quad-SPI flash and 8 MB octal-SPI PSRAM | Confirm populated variant and detected memory during bring-up |
| Header arrangement | Two rows of 22 positions in the product image | Compare every label and orientation on the physical board |
| Other visible features | Two USB-C receptacles, RST/BOOT buttons and RGB LED | USB roles, bridge chip, LED GPIO, regulator and power-path circuit |

Module specifications do not identify the carrier PCB or establish that it uses Espressif's DevKitC-1 circuit. The [Espressif module datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf) defines the N16R8 memory configuration; it does not define this carrier's connectors or regulator capacity.

## Additional board-family references

The [bench-board evidence record](../components/esp32-s3-dev-board/bench-board-reference.md) provides additional front/back product images and annotated port/header references. They corroborate the 44-position map and suggest the YD-style family; the rear reference reads `YD-ESP32-23`, `2022-V1.3`. These are pictured markings, not a verified received revision (Q04).

In the front-view orientation below, the annotations identify left USB-C as native USB/OTG and right USB-C as CH343P USB-to-UART. Port operation and actual routing remain to be checked under Q05. Serial-console access through a UART bridge and native USB/JTAG debugging are distinct functions.

The [selected USB power plan](../components/esp32-s3-dev-board/power-plan.md) adopts the designer's V1.4 circuit as the working family reference: IN-OUT closed exposes the internal USB-fed rail at left row 21 / 5Vin; USB-OTG remains open. Native USB is selected for power and debugging (Q30). The pictured V1.3 revision is not claimed identical; routine rail/current checks remain Q05/Q12.

The [local module datasheet v1.8](../datasheets/esp32-s3-wroom-1_wroom-1u-datasheet-v1.8.pdf) preserves the manufacturer reference for N16R8 memory and module restrictions.

## Header orientation and label map

View the component side, with the antenna at the top and both USB-C receptacles at the bottom, exactly as pictured. Count each row independently from top to bottom. **Left/right and row numbers below are documentation coordinates**, not manufacturer connector designators or chip pin numbers. Looking from the solder side reverses left and right.

![Supplied product image of the N16R8 development board](../../media/reference/gauge-electronics/esp32-s3-development-board.jpg)

Every entry in this table has **Vendor Listing** evidence: it describes the pictured silkscreen, not a continuity-tested signal. The bottom-left power label is transcribed as `5Vin`; confirm the complete label on the actual PCB before using it.

| Row from antenna end | Left label | Right label |
| --- | --- | --- |
| 1 | `3V3` | `GND` |
| 2 | `3V3` | `TX` |
| 3 | `RST` | `RX` |
| 4 | `4` | `1` |
| 5 | `5` | `2` |
| 6 | `6` | `42` |
| 7 | `7` | `41` |
| 8 | `15` | `40` |
| 9 | `16` | `39` |
| 10 | `17` | `38` |
| 11 | `18` | `37` |
| 12 | `8` | `36` |
| 13 | `3` | `35` |
| 14 | `46` | `0` |
| 15 | `9` | `45` |
| 16 | `10` | `48` |
| 17 | `11` | `47` |
| 18 | `12` | `21` |
| 19 | `13` | `20` |
| 20 | `14` | `19` |
| 21 | `5Vin` | `GND` |
| 22 | `GND` | `GND` |

Numeric silkscreen labels are interpreted as GPIO numbers by comparison with Espressif's [DevKitC-1 v1.0 header reference](https://documentation.espressif.com/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.0.html). That reference maps TX/RX to UART0 GPIO43/GPIO44 and RST to the enable/reset function. These are comparison mappings, not confirmation that the purchased carrier is an official DevKitC-1. Verify its traces or board documentation before treating this map as assembly instructions.

## Power, reset and communication interfaces

| Header location / label | Intended function and direction | Domain | Evidence and project use |
| --- | --- | --- | --- |
| Left rows 1-2 / 3V3 | Board-regulated supply output for this design | Nominal 3.3 V | Pictured labels; source selected for the I2C translator low side and compatible peripherals. Verify the two pins' relationship, actual voltage and available current. No direct external 3.3 V injection is selected. |
| Left row 21 / 5Vin | Selected USB-derived output in W01 with IN-OUT closed | Nominal 5 V after USB input diode | Board-family schematic adopted under Q30; feeds sensor, ADC and translator HV. No external supply injection selected; check actual voltage/load during bring-up. |
| Left row 22; right rows 1, 21-22 / GND | Supply return and signal reference | Ground | Pictured labels; verify common return and USB-ground relationships. |
| Left row 3 / RST | Reset/enable input | Board control signal | Comparison reference; confirm reset circuit and active level before external use. Not a spare GPIO. |
| Right row 2 / TX | UART0 transmit, comparison mapping GPIO43 | 3.3 V logic design domain | Reserve for console/programming until onboard bridge routing is established. |
| Right row 3 / RX | UART0 receive, comparison mapping GPIO44 | 3.3 V logic design domain | Reserve for console/programming until onboard bridge routing is established. |
| Right rows 20 / 19 and 19 / 20 | GPIO19 USB D- and GPIO20 USB D+ | Native USB interface | Chip functions documented by Espressif; actual USB-C routing unverified. Preserve for native USB. |
| Other numeric labels | GPIO input/output roles depend on configuration and restrictions below | 3.3 V logic design domain | W01 proposes gauge signals; header exposure does not prove availability. |

The [power architecture](../architecture/power-system.md) defines supply modes. Keep board-derived `3V3` distinct from the shared 5 V sensor/ADC supply and converter output. Display VCC is selected at 3.3 V under Q31, within its listed 3-5 V module range. Regulator margin and simultaneous USB/external-power behavior remain unresolved; selected wiring is a design assumption, not a measured rail.

## GPIO restrictions and allocation policy

Espressif's [GPIO reference](https://docs.espressif.com/projects/esp-idf/en/v5.3.3/esp32s3/api-reference/peripherals/gpio.html) identifies boot-strapping, USB and memory-interface restrictions. The policies below preserve those resources until the board and firmware configuration are known.

| Pins / pictured locations | Constraint | Allocation policy |
| --- | --- | --- |
| GPIO35, 36, 37 / right rows 13, 12, 11 | The N16R8 variant uses octal PSRAM; these module pins serve memory even though exposed on this carrier. | Exclude from gauge wiring. |
| GPIO0, 3, 45, 46 / right row 14, left row 13, right row 15, left row 14 | Boot-strapping pins; external circuits can affect startup. | Avoid for initial peripheral assignments. |
| GPIO19, 20 / right rows 20, 19 | Native USB signals. | Reserve for USB bring-up/debugging. |
| GPIO43, 44 / TX, RX | UART0 comparison mapping; possible bridge connection. | Reserve until programming/console use is resolved. |
| GPIO39-42 / right rows 9-6 | JTAG functions in the DevKitC comparison reference. | Preserve if external JTAG is needed; otherwise evaluate during pin assignment. |
| RGB LED connection | Board-family research identifies GPIO48 as a candidate; the received connection is unverified. | Reserve GPIO48 provisionally; confirm the actual circuit and RGB jumper before reuse. |

The memory restriction follows the [WROOM-1 module specification](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf) and the chip GPIO reference. GPIO33-34 are not shown on these headers; do not infer missing pins from a generic ESP32-S3 pinout. No table here labels unallocated pins as tested or universally safe.

## Gauge interface allocation

| Interface | Required signals | Direction at controller | Assignment |
| --- | --- | --- | --- |
| ADS1115 | SDA, SCL | Bidirectional data; controller-generated clock | Proposed in [W01 controller table](../wiring/bench/generated/connections.md#proposed-controller-assignments); 3.3 V bus to translator; ADC side is 5 V |
| GC9A01 | Clock, MOSI/data, CS, DC, reset | Outputs | Proposed in W01 controller table; module logic/supply checks pending |
| Controls | Deliberate zero and peak reset | Inputs | One multifunction button selected; GPIO proposed in W01 controller table |
| ADC ready notification | ALERT/RDY if selected | Input | W01 proposes polling, leaving ALERT unwired; Q18 pending verification |
| FTP pressure signal | Analog to conditioning and ADS1115 | No direct MCU analog connection | See [measurement chain](../architecture/measurement-system.md) |

Firmware should use explicit GPIO numbers in the bench-board configuration rather than photo row numbers or XIAO aliases. Board environment, flash/PSRAM settings and shared application structure remain in the [firmware plan](../../firmware/README.md). Pinout documentation describes available interfaces; [W01](../wiring/bench/README.md) owns proposed destinations and wiring.

## Verification needed before bench assembly

1. Compare the received board labels/orientation with the listing and adopted family reference during assembly; record the build identity and any discrepancies. New photographs or independent identity authentication are not prerequisites for continuing design.
2. Obtain a schematic for that carrier if available. Verify power/ground/reset connectivity with the board unpowered, and identify regulator, USB bridge and RGB LED connections.
3. Establish supported power modes and measure 3V3 before attaching peripherals. Determine usable regulator current with the board's own consumption and intended loads included.
4. Confirm programming/console port roles, detected module memory and successful boot before using proposed I2C/SPI/control GPIOs.
5. Follow the selected power plan and W01 phase checks; record results and revise the documented assumptions if a concrete discrepancy appears.

The header transcription and cited restrictions have been reviewed as documentation. No continuity checks, power measurements, firmware builds or physical tests have been performed for P01. Purchase details remain in the [BOM](../bom/parts.md).
