# Hardware components

Component-specific descriptions, design decisions, integration notes and import records belong in `hardware/components/<component>/`. Each component or closely related group has a `README.md` as its entry point.

| Component or group | Contents |
| --- | --- |
| [Gauge electronics](gauge-electronics/README.md) | Controllers, display options and shared gauge electronics decisions |
| [ADC](adc/README.md) | ADS1115 modules, conditioning and acquisition requirements |
| [Pressure sensor and pigtail](fuel-tank-pressure-sensor/README.md) | Selected FTP sensor, connector references and calibration prerequisites |
| [Gauge power supply](gauge-power-supply/README.md) | Converter specifications, integration and mounting references |
| [Check valve / relief valve](check-valve-relief-valve/README.md) | Relief component, published specifications and validation needs |

## Adding a component

Create a descriptive lowercase kebab-case folder here and add it to this index. Explain the component's purpose, selected design, source evidence and unresolved integration details. Keep closely related items together when they share a useful record; create additional subfolders only with content that needs them.

Use the shared hardware categories for other artifacts: [architecture](../architecture/README.md), [pinouts](../pinouts/README.md), [wiring](../wiring/README.md), [schematics](../schematics/README.md), [BOM](../bom/parts.md), and [datasheets](../datasheets/README.md). Link to these records from the component page. Purchase inventory belongs in the BOM; native CAD belongs under `cad/`; product reference images remain under `media/reference/<component>/`.

The [hardware index](../README.md) defines the overall organization. Component folders belong here rather than directly under `hardware/`.
