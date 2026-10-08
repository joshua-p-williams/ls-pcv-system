# Hardware components

Component-specific descriptions, design decisions, integration notes and import records belong in `hardware/components/<component>/`. Each component or closely related group has a `README.md` as its entry point.

Read a component page first to understand its job and operating principle, then follow its links to exact interfaces, wiring and tests. The [documentation improvement plan](../../docs/component-documentation-plan.md) tracks the instructional pass and adopted electronics organization.

| Component or group | Contents |
| --- | --- |
| [Gauge electronics](gauge-electronics/README.md) | Integration guide and reading/building path |
| [N16R8 development board](esp32-s3-dev-board/README.md) | Bench controller, memory, power/debugging and board evidence |
| [XIAO ESP32-S3](xiao-esp32s3/README.md) | Compact controller, pin aliases, power and firmware configuration |
| [GC9A01 display](gc9a01-display/README.md) | TFT/SPI concepts, module selection and readability |
| [I2C level shifter](i2c-level-shifter/README.md) | Digital bus translation, purchased module, terminals and pull-ups |
| [RC input filter](rc-input-filter/README.md) | Built resistor/capacitor circuit, operating principle, values and assembly |
| [ADC](adc/README.md) | ADS1115 modules, conditioning and acquisition requirements |
| [Pressure sensor and pigtail](fuel-tank-pressure-sensor/README.md) | Selected FTP sensor, connector references and calibration prerequisites |
| [Gauge power supply](gauge-power-supply/README.md) | Converter specifications, integration and mounting references |
| [Check valve / relief valve](check-valve-relief-valve/README.md) | Relief component, published specifications and validation needs |

## Adding a component

Create a descriptive lowercase kebab-case folder here and add it to this index. A component may be a purchased module or a built subassembly such as the RC input filter; document its function either way. For built circuits, record the internal connectivity and design values, while the BOM distinguishes constituent stock parts from purchases. Explain the component's purpose, selected design, source evidence and unresolved integration details. Keep closely related items together when they share a useful record; create additional subfolders only with content that needs them.

Lead with what the component is, how it works and why the project uses it. Define new terminology and include a practical example when useful. Explain the important interface implications and link to the pinout/harness for exact connections. Label example values as hypothetical, separate published claims from measurements, and keep purchase/import history after the instructional and design material. This approach also applies to mechanical components and future firmware or assembly guides.

Use the shared hardware categories for other artifacts: [architecture](../architecture/README.md), [pinouts](../pinouts/README.md), [wiring](../wiring/README.md), [schematics](../schematics/README.md), [BOM](../bom/parts.md), and [datasheets](../datasheets/README.md). Link to these records from the component page. Purchase inventory belongs in the BOM; native CAD belongs under `cad/`; product reference images remain under `media/reference/<component>/`.

The [hardware index](../README.md) defines the overall organization. Component folders belong here rather than directly under `hardware/`.
