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

On the N16R8 bench setup, initialize the GC9A01 display and read the multifunction button. Provide
a simple visible response to a button action, such as changing a displayed counter or status, and
report useful events over USB serial. Use the documented bench wiring as the starting point and
resolve only the display, input, or board details that materially affect this demonstration. Do
not include sensor acquisition, ADC behavior, calibration, or engine-related logic in this
milestone.

**Why second:** This small vertical slice gives immediate feedback while establishing practical
patterns for a hardware input, a visual output, board configuration, and diagnostics. It lets
those interfaces be exercised before analog measurement and pressure behavior add complexity.

The [M01 brainstorm](../features/M01-interactive-display-button/brainstorm.md) records the
current display, synthetic-data, temporary page-navigation, and research discussion. It is input
to the feature review, not an implementation specification.

## How to use this sequence

This list is an ordering aid, not implementation authorization. Each milestone follows the five
documents in [Developing with Codex](../developing-with-codex/README.md), with review between
documents. Implementation requires approval of the completed documents and explicit selection of
an approved task phase.
