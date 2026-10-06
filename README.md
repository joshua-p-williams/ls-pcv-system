# LS PCV System

Design, document, instrument, and tune a custom crankcase ventilation system for a **2002 Pontiac Trans Am WS6 with a naturally aspirated 408 CID Gen III LS-based stroker** and Holley Terminator X.

This repository is an engineering notebook: mechanical design, hardware, firmware, calibration, and test results belong together. The goal is to select a configuration from measured crankcase pressure.

## System concept

```text
Crankcase -> catch can -> interchangeable fixed restrictor -> manifold vacuum
                 |
                 +-> low-cracking-pressure relief -> atmosphere

Pressure measurement: upstream of the restrictor, referenced to atmosphere
Gauge: FTP sensor -> microcontroller -> dedicated in-cabin display
```

Final hose routing, fresh-air path, and catch-can port assignments remain open.

## Current baseline

These are starting assumptions from the [original project context](docs/LS_PCV_SYSTEM_CONTEXT.md), not validated performance results.

| Area | State |
| --- | --- |
| Catch can | Installed as an earlier project on 2026-09-13 in temporary vented mode; manifold-vacuum PCV remains incomplete |
| Restrictor | 3.0 mm baseline; 2.0 and 4.0 mm comparison variants; initial PAHT-CF print reported complete |
| Relief | Valve ordered with published 0.5 PSI cracking pressure; approximately 0.1 PSI desired, bench validation pending |
| Instrumentation | GM 16238399-style FTP sensor candidate; actual pinout and calibration pending |
| Power | SSLHONG B09NVG35CX converter reported ordered |
| Gauge | MCU, display, wiring, and enclosure not finalized; no firmware yet |
| Repository | Initial documentation plus historical catch-can installation photos; CAD and measured data still need importing |

Primary pressure unit: **inH2O**, relative to atmosphere; negative means vacuum. Approximately -3 to -8 inH2O at hot idle/light cruise is a provisional tuning target, not a GM specification or a validated limit. The 3 mm restrictor is a baseline, not a proven final choice.

## Repository map

| Location | Contents |
| --- | --- |
| [AGENTS.md](AGENTS.md) | Instructions for Codex and other coding assistants |
| [cad/pcv-restrictor/](cad/pcv-restrictor/README.md) | Restrictor design intent and source/export conventions |
| [hardware/bom/parts.md](hardware/bom/parts.md) | Parts, procurement status, and unresolved selections |
| [firmware/](firmware/README.md) | Platform-independent gauge requirements |
| [docs/testing/](docs/testing/test-plan.md) | Planned bench and vehicle validation |
| [docs/status.md](docs/status.md) | Next work and unresolved decisions |
| [Catch-can installation](docs/installation/2026-09-13-catch-can.md) | Earlier vented installation, timeline, and photos |
| [data/](data/README.md) | Calibration and measurement record conventions |
| docs/LS_PCV_SYSTEM_CONTEXT.md | Preserved historical bootstrap context |

Create `cad/gauge/`, `hardware/schematics/`, `hardware/datasheets/`, `docs/research/`, and `media/diagrams/` when there is content to add. Native FreeCAD source belongs in `cad/<component>/source/`; intentional STL/STEP exports belong in `exports/stl/` and `exports/step/`.

## Working in this repository

Start with this README and the relevant component document. Use the original context for background; dated measurements and updated design documents should explain any changes to that baseline. Keep assumptions, published claims, physical observations, and measured results distinct.

Use UTF-8, LF line endings, descriptive lowercase kebab-case names where practical, and explicit units. Preserve native CAD source and raw measurements. For important changes, record the reason, evidence, validation, and unresolved questions alongside the affected component.

There are no dependencies, build commands, or automated tests yet. Review Markdown links, units, and consistency, and run `git diff --check` for tracked changes. New untracked files also need review before the first commit. Add build/test instructions when executable code is introduced.

## Private intake workflow

Place new source material in `_staging/inbox/<batch>/` (for example, a dated project folder). The entire root `_staging/` directory is ignored by Git. Assistants may use `_staging/work/<batch>/` for processing while preserving your inbox originals.

Ask for a batch to be imported. The import should select useful engineering content, remove personal/sensitive information and embedded image metadata, and place reviewed public copies in the appropriate project folders. A public import record should explain provenance, transformations, omissions, and unresolved claims without linking to private files. Originals stay in the inbox unless you request cleanup.

Ignored directories are local only. On a fresh clone, create the inbox with `New-Item -ItemType Directory -Force _staging/inbox` in PowerShell. Git ignore is protection against accidental adds, not encryption or a substitute for reviewing public files; never force-add the staging tree.

## Next steps

1. Import the existing FreeCAD restrictor source and intentional exports.
2. Confirm current parts status and final plumbing; the earlier vented installation is documented separately.
3. Select the MCU/display and verify the actual sensor pinout and electrical interfaces.
4. Characterize the sensor and relief valve on the bench before vehicle tuning.

This design is specific to this engine until evidence supports broader use. The gauge is a diagnostic instrument, not a certified safety device. See the [test plan](docs/testing/test-plan.md) for validation gates.

License selection is pending; this scaffold does not add a license grant.
