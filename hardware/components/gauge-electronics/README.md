# Gauge electronics integration

## How the electronics divide the work

The sensor creates an analog voltage; the ADS1115 measures it; the ESP32 applies calibration, captures peaks and handles the button; the GC9A01 display presents the resulting state. A display refresh must not determine whether a brief pressure event is captured. [A03](../../architecture/measurement-system.md) explains acquisition, peak capture and display smoothing.

Use one controller per build: the N16R8 development board for W01 bench work, or the XIAO for the intended finished gauge and a matching bench build. Shared application code needs explicit board configurations because pins and power paths differ. These selections supersede the earlier Nano/OLED candidates.

## Component guides

| Component | What to learn here | Exact interface |
| --- | --- | --- |
| [N16R8 development board](../esp32-s3-dev-board/README.md) | Chip/module/carrier distinctions, memory, bench access and USB operation | [P01](../../pinouts/esp32-s3-devkit.md) |
| [XIAO ESP32-S3](../xiao-esp32s3/README.md) | Compact-controller role, pin aliases, pin budget and power constraints | [P02](../../pinouts/xiao-esp32s3.md) |
| [GC9A01 TFT](../gc9a01-display/README.md) | Display/controller roles, SPI and readable gauge output | [P05](../../pinouts/gc9a01.md) |
| [I2C level shifter](../i2c-level-shifter/README.md) | BSS138 translation, supply domains and channel assignments | Terminal map in the component guide; connections in W01 |
| [RC input filter](../rc-input-filter/README.md) | Built analog filter, response tradeoff and stock-part assembly | Internal circuit in its component guide; external nodes in W01 |
| [ADS1115](../adc/README.md) | Analog-to-digital conversion, gain, resolution and calibration | [P03](../../pinouts/ads1115.md) |
| [FTP sensor and pigtail](../fuel-tank-pressure-sensor/README.md) | Pressure-to-voltage conversion, connector and atmospheric reference | [P04](../../pinouts/ftp-sensor.md) |
| [Power supply](../gauge-power-supply/README.md) | Buck regulation, rail responsibilities and load budgeting | [P06](../../pinouts/buck-converter.md) |

Procurement status lives in the [BOM](../../bom/parts.md). Component pages preserve selections and relevant product references; this page explains how they fit together. The [bench-board evidence record](../esp32-s3-dev-board/bench-board-reference.md) belongs with the N16R8 component. Existing media locations remain stable.

## Planned measurement and power arrangement

The sensor and ADS1115 share regulated 5 V. The ADC uses the selected +/-6.144 V range without an analog divider. A bidirectional I2C translator separates the ADC's 5 V bus from ESP32 3.3 V logic. The hiBCTR BSS138 translator is selected (Q28); physical bus checks remain Q29. The initial 470 ohm / 1 uF RC filter is selected, with stock-part recording, response and power/protection review under Q17. Display VCC and SPI/control use 3.3 V (Q31), within the listed 3-5 V module supply range. W01 selects native USB with IN-OUT closed for its 5 V branch (Q30); actual rail/load checks remain. A 5 V system rail does not make every signal 5 V compatible.

**I2C** carries ADC data and clock; **SPI** carries display data and clock with separate control signals. The display's SDA/SCL labels denote its SPI interface and must not be confused with the ADC bus. [A01](../../architecture/gauge-system.md) shows these relationships; [A02](../../architecture/power-system.md) defines rail/return responsibilities.

W01 uses one USB source at a time: computer power/debugging or a suitable standalone supply, accepting restart on switching. USB-derived 5 V distribution and load capacity remain Q05/Q12. The SSLHONG converter is the planned vehicle supply and a separate evaluation path, not an instruction to connect two sources together.

## Firmware direction

Planned environment: VS Code, PlatformIO, Arduino framework for ESP32, C/C++. No environment definitions, library versions or firmware implementation exist yet. Keep acquisition, calibration, filtering and peak capture separate from the display driver and board-specific GPIO configuration. The [firmware requirements](../../../firmware/README.md) define the selected single-button interactions and future settings persistence.

Initial bring-up exercises the controller, display/button, ADC and calibrated pressure chain against the complete wiring design. [W01](../../wiring/bench/README.md) derives phased views from its authoritative source; staged testing does not replace planning the final wiring. A XIAO build requires its own complete harness and verification.

## Reading and building path

1. Read [A04](../../architecture/pcv-system.md) to understand the airflow and pressure tap, then [A01](../../architecture/gauge-system.md) for gauge boundaries.
2. Use the component guides above to learn the operating principles and selection tradeoffs.
3. Follow the P01-P06 links for exact module interfaces and orientation conventions; consult [A02](../../architecture/power-system.md) and [A03](../../architecture/measurement-system.md) for power and measurement responsibilities.
4. Review [W01's full diagram and connection schedule](../../wiring/bench/README.md) before its phased assembly steps. Resolve the marked [open questions](../../../docs/open-questions.md) before energizing affected sections.
5. Use the [calibration/test plan](../../../docs/testing/test-plan.md) and [data conventions](../../../data/README.md) to record results. Document generation and successful display output do not establish measured pressure accuracy.

## Import record - 2026-10-06

The original combined import is retained here. Current board and display explanations and reference images are linked from the separate component guides above; procurement remains in the BOM.

Source batch: `guage-electronics` (original spelling retained in private staging). Imported the technical notes into this document, the BOM, firmware README, current status, and agent guidance. Public names use `gauge-electronics`.

| Supplied image | Public image |
| --- | --- |
| `dev-board.jpg` | `esp32-s3-development-board.jpg` |
| `guage-production-board.jpg` | `xiao-esp32s3-pack.jpg` |
| `GC9A01-display.jpg` | `gc9a01-display.jpg` |
| `GC9A01-pin-definition.jpg` | `gc9a01-pin-definition.jpg` |

All four are third-party product-reference images, not photographs of received or tested hardware. Reviewed visible content for personal identifiers; none were observed. Preserved product labels and branding. Removed embedded JPEG application/comment metadata without recompression and verified unchanged orientation, dimensions, and decoded pixels. Public product links omit query parameters and seller-selection identifiers. Inbox originals remain unchanged.

No hardware tests, voltage checks, GPIO assignments, firmware builds, or live supplier specification verification were performed during this documentation import. See the [BOM](../../bom/parts.md) and [project status](../../../docs/status.md) for remaining work.
