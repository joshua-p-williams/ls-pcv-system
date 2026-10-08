# Seeed Studio XIAO ESP32-S3

## Why a second controller board?

The XIAO is the selected compact controller for the finished gauge. It performs the same computation as the [N16R8 bench board](../esp32-s3-dev-board/README.md): acquire ADC readings, apply calibration, capture peaks, handle the button and render the display. Its smaller board suits enclosure planning, while a matching XIAO bench build can exercise the final hardware configuration.

Using the same ESP32-S3 family allows shared application logic. It does not make the two boards interchangeable wire for wire: header positions, available GPIOs and power paths differ. [P02](../../pinouts/xiao-esp32s3.md) defines this board's interface; W01 describes the separate N16R8 harness.

### Pin aliases and a limited pin budget

Labels such as **D4** are board aliases; a **GPIO** number identifies the processor signal used by firmware. A physical header position is a third kind of identifier. Use P02 to relate them instead of assuming identical numbers.

The planned gauge needs two I2C lines, five display lines and one button input: eight GPIOs with the current dedicated-signal approach. Keeping reserved/debug functions available constrains optional additions such as an ADC-ready signal or separate dimming output. A translator changes voltage domains without adding another I2C data/clock pair at the processor. The final XIAO allocation remains separate design work.

## Selected hardware and evidence

Purchased item: standard **Seeed Studio XIAO ESP32-S3**, [Amazon B0DJ6NQFKX](https://www.amazon.com/dp/B0DJ6NQFKX). **Three boards were purchased for USD 21.59 total**. Listed configuration is 8 MB flash / 8 MB PSRAM. Planned roles are the installed gauge, a matching bench controller and a spare/future gauge; these are not completed deployments. Receipt and operation remain unconfirmed. See the [BOM](../../bom/parts.md).

![Vendor XIAO three-pack image](../../../media/reference/gauge-electronics/xiao-esp32s3-pack.jpg)

The [original electronics import record](../gauge-electronics/README.md#import-record---2026-10-06) describes the image's provenance. It does not identify the received board revision. Sense expansion hardware and the Plus variant are outside this selection. P02 links applicable manufacturer references.

## Power and interfaces

The sensor and ADS1115 share 5 V; the controller's I2C signals remain 3.3 V through the bidirectional translator. The SPI display is a separate interface, and its module VCC is selected at 3.3 V; XIAO rail capacity remains a separate check. The FTP signal goes to the external ADC, not directly to a XIAO analog input.

USB power, the 5V/VBUS header and the onboard 3.3 V regulator have different responsibilities. A converter's advertised 3 A capacity does not establish the current available from the XIAO regulator. P02 records the conflicting published 600/700 mA references and USB-linked VBUS arrangement. Q13 and the load budget must be settled for this build; N16R8 bench results do not verify XIAO power behavior.

Battery support is listed, but battery operation is not selected for this gauge. Do not treat battery pads as a 5 V input or assume the board prevents backfeeding a connected USB host.

## Firmware and learning path

The [firmware requirements](../../../firmware/README.md#board-configurations) call for explicit board configurations and shared measurement/UI logic. Flash stores the program and persisted content; RAM/PSRAM holds temporary working state. Calibration and user settings need an explicit persistent-storage implementation.

Start with the [gauge integration guide](../gauge-electronics/README.md), then P02 for exact interfaces and [A02](../../architecture/power-system.md) for supply responsibilities. A future XIAO harness must document complete wiring and phased bring-up against this board's own power and pin configuration.
