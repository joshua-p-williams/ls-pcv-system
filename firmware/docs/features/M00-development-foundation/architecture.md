# Architecture: M00 — Development Foundation

## Overview

M00 establishes a small PlatformIO project under `firmware/`, using the Arduino framework on
the ESP32-S3 N16R8 development board. The project pins the official Espressif 32 PlatformIO
platform to version 8.0.0. A project-local board profile describes the selected N16R8 module
configuration and native USB serial path instead of treating PlatformIO's N8 default as an exact
match.

The application is a minimal Arduino program: it starts USB serial reporting when available and
emits a low-rate heartbeat from the main loop without waiting for a monitor. Setup and verification
instructions live with the firmware, with native Windows and Linux paths and equivalent VS Code
and CLI workflows.

## What gets built and how it fits together

The PlatformIO project root is `firmware/`, alongside the firmware README and documentation that
already live there. Its initial structure is:

```text
firmware/
├── platformio.ini
├── boards/
│   └── esp32-s3-n16r8.json
├── src/
│   └── main.cpp
├── README.md
└── docs/
    ├── development-environment.md
    ├── developing-with-codex/
    ├── features/
    └── milestones/
```

This follows PlatformIO's conventional project layout. A separate project-name directory is not
needed for the single firmware application in this repository. If the repository later gains a
genuinely separate firmware application with its own build and release lifecycle, it can receive
its own PlatformIO root then. Board variants such as N16R8 and XIAO belong in named PlatformIO
environments sharing application source, rather than duplicated project trees.

- `firmware/platformio.ini` defines the Arduino environment, pins `espressif32@8.0.0`, selects the
  project-local N16R8 board profile, and sets the native USB CDC options required for the W01
  programming and diagnostic connection.
- `firmware/boards/esp32-s3-n16r8.json` describes the module-level target assumptions: ESP32-S3,
  16 MB flash, and 8 MB Octal-SPI PSRAM. It is a project build profile, not a claim that the
  carrier-board revision has been physically identified. Its fields must be checked against
  PlatformIO 8.0.0's board schema during implementation.
- `firmware/src/main.cpp` contains only startup reporting and a periodic heartbeat. It does not
  initialize the display, button, ADC, or sensor.
- `firmware/docs/development-environment.md` explains prerequisites, project setup, build, upload,
  serial monitoring, and M00 verification on native Windows and Linux. It provides VS Code steps
  and matching CLI commands, and records only OS-specific USB access steps that are actually
  observed.
- `firmware/README.md` provides a short entry point and links to the setup guide. The root README
  links to the firmware entry point without repeating its setup procedure.

PlatformIO's build output is local and remains uncommitted. The setup guide records the selected
platform and framework versions so contributors can compare their environment with the tested
configuration.

## Key decisions and why

### Pin the official Espressif 32 platform at 8.0.0

`platformio.ini` pins `espressif32@8.0.0`; the framework version is supplied by that platform
release (Arduino-ESP32 3.3.12). This is the selected official release and no project-specific
reason has emerged to start on the older 7.1.x line. M00 verifies the selected combination on both
host operating systems before documenting it as tested.

### Define the N16R8 target explicitly

PlatformIO's built-in `esp32-s3-devkitc-1` profile describes an N8 configuration with 8 MB flash
and no PSRAM. M00 instead targets the project's ESP32-S3-WROOM-1-N16R8 module. A project-local
profile makes the 16 MB flash and 8 MB PSRAM assumptions visible and reviewable, while keeping the
carrier identity and physical USB behavior as matters for the stated verification checks. The
profile does not create a second application or imply automatic board detection.

### Use native USB Serial/JTAG for upload and diagnostics

W01 selects the development board's native USB connection and reserves GPIO19/GPIO20 for it. The
build enables the Arduino USB CDC path so `Serial` uses that interface. Startup reporting does not
wait for host enumeration or a monitor; a periodic heartbeat lets a monitor opened after startup
confirm that the program remains active. USB serial is diagnostic output, not a runtime dependency.

### Keep the first project structure conventional and small

The initial source stays in `src/main.cpp`, with target and dependency configuration in
`platformio.ini` and the project board profile under `boards/`. No application layers, generic board
abstraction, local library, or test framework is introduced before a real feature needs one. M01
will be the first opportunity to select and apply a meaningful automated-test approach for
button-to-display behavior.

### Keep setup documentation with the firmware

Firmware contributors need the detailed PlatformIO, USB, build, upload, and monitor instructions
near the code they are using. The root README remains project-wide navigation; it links to the
firmware entry point instead of duplicating host-specific setup steps.

## What's explicitly out of scope

- XIAO ESP32-S3 build profile or deployment; add it when a milestone needs firmware on that board
  and its pin mapping has been established.
- Display, button, sensor, ADS1115, calibration, gauge behavior, and Wokwi firmware simulation.
- WSL USB passthrough, automatic board detection, and support for multiple carrier revisions.
- A unit-test framework or placeholder test. M00 verifies builds and the minimal program on
  hardware; meaningful automated tests are considered with M01.
- Any claim that building, uploading, or receiving serial output validates protoboard wiring,
  peripheral electrical behavior, pressure readings, or vehicle operation.
