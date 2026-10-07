# LS PCV System

Design, document, instrument, and tune a custom crankcase ventilation system for a **2002 Pontiac Trans Am WS6 with a naturally aspirated 408 CID Gen III LS-based stroker** and Holley Terminator X.

This repository is an engineering notebook: mechanical design, hardware, firmware, calibration, and test results belong together. The goal is to select a configuration from measured crankcase pressure.

## System concept

```text
Crankcase -> catch can -> interchangeable fixed restrictor -> manifold vacuum
                 |
                 +-> low-cracking-pressure relief -> atmosphere

Pressure measurement: upstream of the restrictor, referenced to atmosphere
Gauge: FTP sensor -> conditioning -> ADS1115 -> ESP32-S3 -> GC9A01 display
```

The [engine PCV overview](hardware/architecture/pcv-system.md) records the selected fresh-air and evacuation paths. Exact ports, fittings, physical hose routing and sensor mounting remain open.

## Current baseline

The baseline combines reported installation and purchase information, imported artifacts, and current design choices. Purchased and modeled components have not yet established measured system performance. The [original project context](docs/LS_PCV_SYSTEM_CONTEXT.md) preserves earlier reasoning and superseded proposals.

| Area | State |
| --- | --- |
| Catch can | Installed as an earlier project on 2026-09-13 in temporary vented mode; manifold-vacuum PCV remains incomplete |
| Restrictor | 3.0 mm baseline; 2.0 and 4.0 mm comparison variants; initial PAHT-CF print reported complete |
| Relief | Three EVIL ENERGY valves purchased; advertised 0.5 PSI opening pressure; approximately 0.1 PSI desired, modification/bench validation pending |
| Instrumentation | GM 16238399-style FTP sensor and HiSport 13585316 pigtail purchased; receipt, actual pinout, fit, and calibration pending |
| Power | [SSLHONG B09NVG35CX](hardware/components/gauge-power-supply/README.md) purchased for USD 13.99; advertised 8-60 V input, 5 V / 3 A USB-C output; validation pending |
| Gauge | ESP32-S3 bench/XIAO boards, GC9A01 TFTs, and ADS1115 modules purchased; ADC conditioning, wiring, final display layout, and enclosure pending; no firmware yet |
| Repository | Documentation, catch-can photos, native restrictor CAD, 3MF variants, and model views imported; measured data still pending |

Primary pressure unit: **inH2O**, relative to atmosphere; negative means vacuum. Approximately -3 to -8 inH2O at hot idle/light cruise is a provisional tuning target, not a GM specification or a validated limit. The 3 mm restrictor is a baseline, not a proven final choice.

## Repository map

| Location | Contents |
| --- | --- |
| [AGENTS.md](AGENTS.md) | Instructions for Codex and other coding assistants |
| [Hardware documentation](hardware/README.md) | Architecture, interfaces, wiring and component navigation |
| [Diagram inventory](hardware/DIAGRAMS.md) | Existing drafts and proposed diagrams/interface pages |
| [A04: engine PCV overview](hardware/architecture/pcv-system.md) | Planned airflow, relief, restrictor and pressure tap |
| [A01: overall gauge architecture](hardware/architecture/gauge-system.md) | Planned measurement, controller, display and power boundaries |
| [A02: power architecture](hardware/architecture/power-system.md) | Supply paths, returns and bench/vehicle/USB modes |
| [A03: measurement signal chain](hardware/architecture/measurement-system.md) | Calibration, validity, peak capture and display smoothing |
| [Diagramming standard](docs/diagramming-and-wiring-standard.md) | Formats, folder responsibilities, evidence and tooling |
| [cad/pcv-restrictor/](cad/pcv-restrictor/README.md) | Restrictor design intent and source/export conventions |
| [Hardware components](hardware/components/README.md) | Component descriptions, design decisions and import records |
| [hardware/bom/parts.md](hardware/bom/parts.md) | Parts, procurement status, and unresolved selections |
| [Gauge electronics](hardware/components/gauge-electronics/README.md) | Selected controllers, initial display, and planned interfaces |
| [Pressure sensor and pigtail](hardware/components/fuel-tank-pressure-sensor/README.md) | Purchased parts, connector references, and verification needs |
| [ADS1115 acquisition](hardware/components/adc/README.md) | ADC selection, preliminary conditioning, and sampling plan |
| [Gauge power supply](hardware/components/gauge-power-supply/README.md) | Converter selection, power plan, and mounting reference |
| [Check valve / relief](hardware/components/check-valve-relief-valve/README.md) | Purchased valve, vendor claims, and relief-validation needs |
| [Datasheets](hardware/datasheets/README.md) | Preserved vendor revisions and official source links |
| [firmware/](firmware/README.md) | Gauge requirements and planned ESP32-S3 development environment |
| [docs/testing/](docs/testing/test-plan.md) | Planned bench and vehicle validation |
| [Open questions](docs/open-questions.md) | Follow-up questions, evidence needed and bench-harness dependencies |
| [docs/status.md](docs/status.md) | Next work and unresolved decisions |
| [Catch-can installation](docs/installation/2026-09-13-catch-can.md) | Earlier vented installation, timeline, and photos |
| [data/](data/README.md) | Calibration and measurement record conventions |
| [Original context](docs/LS_PCV_SYSTEM_CONTEXT.md) | Historical rationale; superseded choices are not current instructions |

Create `cad/gauge/` and `docs/research/` with their first useful content. Native FreeCAD source belongs in `cad/<component>/source/`; the current restrictor 3MF variants are together in `cad/pcv-restrictor/printing/`. Add intentional STL/STEP exports under a component's `exports/` if needed. Keep installation photos in `media/photos/`, model views/diagrams in `media/diagrams/`, and vendor illustrations in `media/reference/`.

## Working in this repository

Start with this README and the relevant component document. Use the original context for background; dated measurements and updated design documents should explain any changes to that baseline. Keep assumptions, published claims, physical observations, and measured results distinct.

Use the BOM for current purchase/inventory facts, component READMEs for current design details, and `docs/status.md` for open work. Dated installation/import records preserve what was observed or imported at that time. Native CAD defines saved geometry; physical records define what was actually built and measured. Reconcile conflicts explicitly instead of assuming the historical context or a filename is authoritative for every question.

Use UTF-8, LF line endings, descriptive lowercase kebab-case names where practical, and explicit units. Preserve native CAD source and raw measurements. For important changes, record the reason, evidence, validation, and unresolved questions alongside the affected component.

There are no firmware dependencies, build commands, or automated tests yet. Review Markdown links, units, and consistency, and run `git diff --check` for unstaged changes and `git diff --cached --check` for staged changes. Review new untracked files separately; before committing, ensure the staged versions include the intended edits. Add build/test instructions when executable code is introduced.

## Private intake workflow

Place new source material in `_staging/inbox/<batch>/` (for example, a dated project folder). The entire root `_staging/` directory is ignored by Git. Assistants may use `_staging/work/<batch>/` for processing while preserving your inbox originals.

Ask for a batch to be imported. The import should select useful engineering content, remove personal/sensitive information and embedded image metadata, and place reviewed public copies in the appropriate project folders. A public import record should explain provenance, transformations, omissions, and unresolved claims without linking to private files. Originals stay in the inbox unless you request cleanup.

Ignored directories are local only. On a fresh clone, create the inbox with `New-Item -ItemType Directory -Force _staging/inbox` in PowerShell. Git ignore is protection against accidental adds, not encryption or a substitute for reviewing public files; never force-add the staging tree.

## Next steps

1. Verify physical variants, measured bores, and slicer behavior against the [adopted restrictor CAD design](cad/pcv-restrictor/README.md).
2. Confirm current parts status and final plumbing; the earlier vented installation is documented separately.
3. Finalize ADS1115 conditioning/configuration and verify sensor, board, and display electrical interfaces before bringing up the ESP32-S3/GC9A01 prototype.
4. Characterize the sensor and relief valve on the bench before vehicle tuning.

This design is specific to this engine until evidence supports broader use. The gauge is a diagnostic instrument, not a certified safety device. See the [test plan](docs/testing/test-plan.md) for validation gates.

License selection is pending; this scaffold does not add a license grant.
