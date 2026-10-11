# Original visual parts

These definitions provide named terminals for parts lacking a selected native Wokwi representation. They are **illustrations, not functioning models**. The original `-visual` bodies and leg order are not real shapes or connector positions.

| Exact online chip name | Definition | Inert stub | Represents |
| --- | --- | --- | --- |
| `pcv-ads1115-visual` | [JSON](pcv-ads1115-visual.chip.json) | [C](pcv-ads1115-visual.chip.c) | J3, including unused inputs/ALERT |
| `pcv-gc9a01-visual` | [JSON](pcv-gc9a01-visual.chip.json) | [C](pcv-gc9a01-visual.chip.c) | J2 seven-terminal SPI display |
| `pcv-level-shifter-visual` | [JSON](pcv-level-shifter-visual.chip.json) | [C](pcv-level-shifter-visual.chip.c) | LS1 four pairs, references and GND aliases |
| `pcv-ftp-visual` | [JSON](pcv-ftp-visual.chip.json) | [C](pcv-ftp-visual.chip.c) | FTP A/B/C interface |
| `pcv-rails-visual` | [JSON](pcv-rails-visual.chip.json) | [C](pcv-rails-visual.chip.c) | Separate 3V3, GND and 5V nodes |
| `pcv-capacitor-visual` | [JSON](pcv-capacitor-visual.chip.json) | [C](pcv-capacitor-visual.chip.c) | C1 OUT/GND leads; 1 uF nonpolar, at least 10 V |
| `pcv-ads1115-header` | [JSON](pcv-ads1115-header.chip.json) | [C](pcv-ads1115-header.chip.c) | L03 single ten-position component-side header from P03 |
| `pcv-level-shifter-header` | [JSON](pcv-level-shifter-header.chip.json) | [C](pcv-level-shifter-header.chip.c) | L03 parallel six-position component-side headers from the module reference |

The two `-header` symbols are rotated 270 degrees in L03 to reproduce documented terminal order. Empty positions put the ADC contacts on one edge. Their body dimensions and separation between rows remain Wokwi graphics, not measured footprints. Their stubs have no behavior. L01 retains its original functional symbols.

Create the exact names using the [Custom Chip workflow](https://docs.wokwi.com/chips-api/getting-started), paste matching files, then the main [diagram.json](../diagram.json). Online filenames omit `chips/`; diagram types add `chip-`. Definitions follow the [pin format](https://docs.wokwi.com/chips-api/chip-json); empty entries leave blank symbol positions.

The original stubs only print a notice from `chip_init()`. They do not drive pins, respond to protocols, join internal nets or produce measurements. Repeated terminals represent external nets; the stub is not an electrical board model. RAILS supplies stay separate, and both LS1 ground pads are externally wired to common return.

Use real pinout/component references from the [main guide](../README.md) for orientation. The sensor symbol is not a mating-face view; capacitor labels identify nodes without implying polarity. R1/C1 show topology without validating response.

For future functional models, record source/license, behavior, omissions and verification here. No community implementation, compiled WebAssembly or gauge firmware is bundled.
