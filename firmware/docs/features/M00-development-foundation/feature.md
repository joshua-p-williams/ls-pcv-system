# Feature: Reproducible Firmware Development Foundation

## What is this feature?

This milestone delivers both a dependable firmware development environment and the instructions
for setting it up. A contributor can follow the repository's native Windows or native Linux guide,
open the project in VS Code, build and upload the first firmware to the ESP32-S3 N16R8 development
board, and observe a minimal program report that it has started and remains active.

## Why it matters

Later firmware depends on a working development loop. Establishing and documenting that loop first
makes toolchain, board-upload, and serial-monitor issues visible before display, button, and
pressure-measurement behavior are added. Reproducible instructions also make the repository useful
to someone learning or continuing the project outside the original development environment.

## What "done" looks like

- The PlatformIO project is rooted in `firmware/` and has a small, conventional structure that can
  grow with real application needs.
- The project builds and uploads to the N16R8 development board on native Windows and native
  Linux using the documented setup.
- USB serial output shows a startup message and a periodic heartbeat after upload on both
  operating systems.
- The firmware does not wait indefinitely for a serial monitor and continues running when no
  computer or monitor is attached.
- The verified PlatformIO platform and framework versions are pinned in the project configuration
  and identified in the setup documentation.
- VS Code with the PlatformIO extension is documented as the guided workflow, with equivalent CLI
  commands provided for building, uploading, and monitoring serial output.
- A firmware-specific development-environment guide documents setup and verification for native
  Windows and Linux. `firmware/README.md` links to it, and the root project README points readers
  toward the firmware documentation without duplicating setup steps.
- The documentation distinguishes successful software setup from electrical or functional
  validation of the protoboard and peripherals.

## What this feature is deliberately not

This milestone does not implement the GC9A01 display, button behavior, sensor or ADS1115
acquisition, calibration, pressure calculations, gauge logic, XIAO deployment, or Wokwi firmware
simulation. It does not require WSL USB passthrough, a unit-test framework, or a comprehensive
coding-standards manual. Meaningful automated tests will be considered with M01's button-to-display
behavior; hardware checks remain separate from software tests.
