# ESP32-S3 N16R8 bench-board interface

P01 documents the bench controller used by the [gauge architecture](../architecture/gauge-system.md). The header map below is transcribed from the supplied product image. Electrical functions are qualified against Espressif references where applicable; the received board, PCB revision and wiring have not been verified. Project GPIO assignments remain open.

## Board identity and evidence

| Item | Available evidence | Remaining check |
| --- | --- | --- |
| Purchase identifier | Amazon ASIN B0D93DLB6Q; see [component record](../components/gauge-electronics/README.md) | Identify actual PCB manufacturer and revision |
| Module marking in product image | `ESP32-S3-WROOM-1`, `N16R8` | Compare with the received module |
| Memory configuration | Espressif identifies WROOM-1-N16R8 as 16 MB quad-SPI flash and 8 MB octal-SPI PSRAM | Confirm populated variant and detected memory during bring-up |
| Header arrangement | Two rows of 22 positions in the product image | Compare every label and orientation on the physical board |
| Other visible features | Two USB-C receptacles, RST/BOOT buttons and RGB LED | USB roles, bridge chip, LED GPIO, regulator and power-path circuit |

Module specifications do not identify the carrier PCB or establish that it uses Espressif's DevKitC-1 circuit. The [Espressif module datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf) defines the N16R8 memory configuration; it does not define this carrier's connectors or regulator capacity.

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
| Left rows 1-2 / 3V3 | Board-regulated supply output for this design | Nominal 3.3 V | Pictured labels; source selected for ADS1115 and compatible peripherals. Verify the two pins' relationship, actual voltage and available current. No direct external 3.3 V injection is selected. |
| Left row 21 / 5Vin | Candidate external board power input | Nominal 5 V, pending board confirmation | Pictured label; verify power path and USB interaction before using the converter here. |
| Left row 22; right rows 1, 21-22 / GND | Supply return and signal reference | Ground | Pictured labels; verify common return and USB-ground relationships. |
| Left row 3 / RST | Reset/enable input | Board control signal | Comparison reference; confirm reset circuit and active level before external use. Not a spare GPIO. |
| Right row 2 / TX | UART0 transmit, comparison mapping GPIO43 | 3.3 V logic design domain | Reserve for console/programming until onboard bridge routing is established. |
| Right row 3 / RX | UART0 receive, comparison mapping GPIO44 | 3.3 V logic design domain | Reserve for console/programming until onboard bridge routing is established. |
| Right rows 20 / 19 and 19 / 20 | GPIO19 USB D- and GPIO20 USB D+ | Native USB interface | Chip functions documented by Espressif; actual USB-C routing unverified. Preserve for native USB. |
| Other numeric labels | GPIO input/output roles depend on configuration and restrictions below | 3.3 V logic design domain | No gauge signal is assigned yet; header exposure does not prove availability. |

The [power architecture](../architecture/power-system.md) defines supply modes. Keep board-derived `3V3` distinct from the 5 V sensor supply and converter output. The display supply, regulator margin and simultaneous USB/external-power behavior remain unresolved. This page assigns no load to an unverified power pin.

## GPIO restrictions and allocation policy

Espressif's [GPIO reference](https://docs.espressif.com/projects/esp-idf/en/v5.3.3/esp32s3/api-reference/peripherals/gpio.html) identifies boot-strapping, USB and memory-interface restrictions. The policies below preserve those resources until the board and firmware configuration are known.

| Pins / pictured locations | Constraint | Allocation policy |
| --- | --- | --- |
| GPIO35, 36, 37 / right rows 13, 12, 11 | The N16R8 variant uses octal PSRAM; these module pins serve memory even though exposed on this carrier. | Exclude from gauge wiring. |
| GPIO0, 3, 45, 46 / right row 14, left row 13, right row 15, left row 14 | Boot-strapping pins; external circuits can affect startup. | Avoid for initial peripheral assignments. |
| GPIO19, 20 / right rows 20, 19 | Native USB signals. | Reserve for USB bring-up/debugging. |
| GPIO43, 44 / TX, RX | UART0 comparison mapping; possible bridge connection. | Reserve until programming/console use is resolved. |
| GPIO39-42 / right rows 9-6 | JTAG functions in the DevKitC comparison reference. | Preserve if external JTAG is needed; otherwise evaluate during pin assignment. |
| RGB LED connection | A visible onboard LED does not identify its GPIO. Board variants differ. | Confirm its connection before allocating any potentially shared GPIO. Do not copy another board's LED pin definition. |

The memory restriction follows the [WROOM-1 module specification](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf) and the chip GPIO reference. GPIO33-34 are not shown on these headers; do not infer missing pins from a generic ESP32-S3 pinout. No table here labels unallocated pins as tested or universally safe.

## Gauge interface allocation

| Interface | Required signals | Direction at controller | Assignment |
| --- | --- | --- | --- |
| ADS1115 | SDA, SCL | Bidirectional data; controller-generated clock | GPIOs TBD; planned 3.3 V bus, verify pull-ups |
| GC9A01 | Clock, MOSI/data, CS, DC, reset | Outputs | GPIOs TBD; module logic/supply checks pending |
| Controls | Deliberate zero and peak reset | Inputs | Button count and GPIOs TBD |
| ADC ready notification | ALERT/RDY if selected | Input | Optional; polling versus ready notification unresolved |
| FTP pressure signal | Analog to conditioning and ADS1115 | No direct MCU analog connection | See [measurement chain](../architecture/measurement-system.md) |

Firmware should use explicit GPIO numbers in the bench-board configuration rather than photo row numbers or XIAO aliases. Board environment, flash/PSRAM settings and shared application structure remain in the [firmware plan](../../firmware/README.md). Pinout documentation describes available interfaces; the future bench harness will own actual destinations and wiring.

## Verification needed before the bench harness

1. Record clear front/back views, module marking, PCB marking/revision and USB labels of the received board; compare all 44 header positions with the image map.
2. Obtain a schematic for that carrier if available. Verify power/ground/reset connectivity with the board unpowered, and identify regulator, USB bridge and RGB LED connections.
3. Establish supported power modes and measure 3V3 before attaching peripherals. Determine usable regulator current with the board's own consumption and intended loads included.
4. Confirm programming/console port roles, detected module memory and successful boot before selecting I2C/SPI/control GPIOs.
5. Record results and any corrections here, then define the [bench harness](../wiring/README.md) from verified interfaces.

The header transcription and cited restrictions have been reviewed as documentation. No continuity checks, power measurements, firmware builds or physical tests have been performed for P01. Purchase details remain in the [BOM](../bom/parts.md).
