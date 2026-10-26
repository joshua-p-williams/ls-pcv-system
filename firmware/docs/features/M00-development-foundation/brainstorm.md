# M00 — Development Foundation Brainstorm

This note captures the agreed direction and questions to resolve for the development-foundation
milestone. It is input to the feature definition, not a feature specification or authorization to
implement. Review and revise it before drafting `feature.md`.

## Desired outcome

A contributor can open the firmware project in VS Code, build and upload a minimal program to the
ESP32-S3 N16R8 development board, and observe that it is running through USB serial diagnostics.
The setup should be understandable from the repository and repeatable on native Windows and
native Linux.

## Agreed direction

- `firmware/` is the PlatformIO project root; firmware remains one part of the larger hardware
  repository.
- Use the VS Code PlatformIO extension as the guided workflow. Document equivalent PlatformIO CLI
  commands for repeatability and terminal-based use.
- Keep the detailed native Windows/Linux setup guide in `firmware/docs/development-environment.md`.
  Use `firmware/README.md` as the firmware entry point with a concise getting-started summary and
  link; keep the root `README.md` project-wide and link to firmware rather than duplicating setup
  steps.
- Support native Windows and native Linux. Verify build, upload and serial-monitor behavior in
  both environments using the same project configuration and N16R8 target.
- WSL is optional and is not an M00 acceptance environment. The instructions should not depend on
  WSL or USB passthrough.
- Build and upload only for the N16R8 in M00. Keep a future XIAO configuration possible, but add
  and validate it when a milestone first needs to run firmware on that board. Do not guess its
  pin assignments.
- Follow mainstream, tool-supported practices and keep the structure proportional to current
  needs. Explain a real early design fork when it affects later work; avoid speculative frameworks
  and process documents.
- Use the conventional PlatformIO layout. Begin with the project configuration and a small
  `src/main.cpp`; add modules, headers, local libraries and test structure when actual code gives
  them a purpose. Keep the application from accumulating board-specific pin assumptions as
  peripherals are introduced.
- Pin the PlatformIO platform/framework version used for the verified build. Select exact versions
  during research and verify that the same project configuration works on both operating systems.
- The minimal program prints a startup message and a periodic heartbeat over the N16R8's selected
  native USB serial connection. Schedule the heartbeat without a long blocking delay, and do not
  wait indefinitely for a host or serial monitor; the firmware must continue running when USB
  serial is not being monitored.
- M00 verification covers build, upload and runtime serial behavior on both operating systems.
  Do not add a dummy unit test solely to prove a test runner starts. Select and exercise a test
  approach with M01's meaningful button-to-display behavior; verify hardware behavior separately.

## M00 boundaries

M00 establishes the project and development loop. It does not implement the display, button,
sensor/ADC acquisition, calibration, pressure calculations, gauge behavior, XIAO deployment or
Wokwi firmware simulation. It does not create a broad coding-standards manual. Later work should
extend the project using the conventions needed by real features.

## Research to resolve for the feature documents

- Identify the correct PlatformIO board definition and required configuration for the N16R8 board
  and W01's native USB serial/programming path.
- Select and pin a compatible ESP32 platform/framework version, then confirm build and upload
  behavior on native Windows and native Linux.
- Document VS Code/PlatformIO setup, CLI alternatives, serial-port selection and any OS-specific
  driver or permission steps based on the actual board interface.
- Define observable build/upload/serial checks and distinguish successful software setup from
  electrical validation of the protoboard.
- Choose a simple heartbeat cadence and startup text that demonstrate operation without adding
  behavior that belongs to a later milestone.

## Relationship to the next milestone

M01 is the agreed interactive display/button demonstration on the N16R8. It should introduce a
test framework only alongside meaningful button-to-display logic, with hardware checks kept
separate from automated tests. The exact behavior and dependencies belong in M01's own review
cycle.
