# Firmware milestones

This folder records the agreed order of firmware milestones. The milestone list is the roadmap;
the review artifacts for each milestone live in `../features/<ID>-<short-slug>/` and follow the
five-document workflow in [Developing with Codex](../developing-with-codex/README.md).

## Agreed initial sequence

The first two milestones establish a usable development workflow and then exercise a simple,
visible hardware interaction. The sequence is intentionally incomplete: add later milestones
after these foundations provide practical feedback about the project and hardware.

### M00 — Development foundation

Establish a maintainable firmware project layout and a reproducible VS Code/PlatformIO workflow,
using pragmatic conventions only where the scaffold needs them. Demonstrate the workflow by
building and uploading a minimal program to the N16R8 development board and observing USB serial
diagnostics. Document setup, build, upload, and verification steps for instructional use. Keep the
structure ready for board-specific configurations, including the XIAO target, without inventing
unresolved pin assignments or expanding this into a broad standards project.

**Why first:** Every later feature depends on a reliable way to build, flash, observe, and explain
the firmware. Proving that loop early exposes toolchain or USB issues before they are entangled
with sensor and display behavior.

The [M00 brainstorm](../features/M00-development-foundation/brainstorm.md) preserves the initial
discussion that informed the specification. The [M00 feature](../features/M00-development-foundation/feature.md)
defines its outcome and scope, and [M00 research](../features/M00-development-foundation/research.md)
records the evidence and selected toolchain. The [M00 architecture](../features/M00-development-foundation/architecture.md)
describes the project structure and runtime design, and the [M00 plan](../features/M00-development-foundation/plan.md)
orders implementation and verification work. The [M00 tasks](../features/M00-development-foundation/tasks.md)
provide the phase checklist for implementation after the documents are approved.

### M01 — Interactive display and button

On the N16R8 bench setup, demonstrate the gauge interface using a physical GC9A01 and multifunction
button. The implemented version displays signed synthetic pressure, status colors, a one-second
30-sample trend, and current-run extrema; temporary gestures change demo values and cycle the pages.
It does not include sensor acquisition, ADC behavior, calibration, or engine-related logic.

**Why second:** This small vertical slice gives immediate feedback while establishing practical
patterns for a hardware input, a visual output, board configuration, and diagnostics. It lets
those interfaces be exercised before analog measurement and pressure behavior add complexity.

The [M01 brainstorm](../features/M01-interactive-display-button/brainstorm.md) preserves the
display, synthetic-data, temporary page-navigation, and research discussion. The
[M01 feature definition](../features/M01-interactive-display-button/feature.md) defines scope and
acceptance criteria, and [M01 tasks](../features/M01-interactive-display-button/tasks.md) record
implementation and physical verification. Integrated N16R8 checks are complete; Phase 6 closeout
is in progress, with native Linux host tests/build/upload still pending.

### M02 — Sensor-to-display measurement path

Incrementally validate the ADS1115 voltage path, characterize the FTP sensor against pressure
references, and display acquired pressure readings. Begin with measured battery voltages, then use
the planned water-column U-tube and the purchased digital manometer for controlled sensor tests.
The approved scope, acceptance criteria, and phased tasks are in the
[M02 feature documents](../features/M02-sensor-to-display/feature.md) and
[task list](../features/M02-sensor-to-display/tasks.md). Phase 3 voltage-only bench verification is
complete. Phase 4 now has a compiled live N16R8 composition and fault-aware three-page rendering;
the atmospheric sensor/ADC check is recorded, while controlled pressure response and on-device
display/fault checks remain.

**Why next:** M01 established the display and button with synthetic data. Replacing the synthetic
source with a measured signal is the next useful vertical slice, and staged electrical and pressure
checks can isolate acquisition, conversion, and display issues before vehicle testing.

## How to use this sequence

This list is an ordering aid, not implementation authorization. Each milestone follows the five
documents in [Developing with Codex](../developing-with-codex/README.md), with review between
documents. Implementation requires approval of the completed documents and explicit selection of
an approved task phase.
