# Research: M00 — Development Foundation

This document records the project evidence and published toolchain information relevant to M00.
It separates documented design facts from hardware behavior that still needs build and bench
verification. Platform version information below was checked on 2026-10-23 and should be checked
again before implementation because these packages change over time.

## Prior art reviewed

- Project firmware requirements and board direction: [`firmware/README.md`](../../../README.md),
  [N16R8 component record](../../../../hardware/components/esp32-s3-dev-board/README.md),
  [bench-board reference record](../../../../hardware/components/esp32-s3-dev-board/bench-board-reference.md),
  [P01 pinout](../../../../hardware/pinouts/esp32-s3-devkit.md), and
  [W01 bench wiring](../../../../hardware/wiring/bench/README.md).
- [PlatformIO IDE for VS Code](https://docs.platformio.org/en/stable/integration/ide/vscode.html),
  [PlatformIO Core requirements](https://docs.platformio.org/en/latest/core/installation/requirements.html),
  [project initialization and layout](https://docs.platformio.org/en/stable/userguide/project/cmd_init.html),
  and [platform version pinning](https://docs.platformio.org/en/latest/projectconf/sections/env/options/platform/platform.html).
- PlatformIO's [Espressif 32 release history](https://github.com/platformio/platform-espressif32/releases),
  [ESP32-S3-DevKitC-1 board entry](https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html),
  and the [v8.0.0 board manifest](https://raw.githubusercontent.com/platformio/platform-espressif32/v8.0.0/boards/esp32-s3-devkitc-1.json).
- Espressif's [ESP32-S3-WROOM-1 datasheet](https://documentation.espressif.com/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf),
  [Arduino-ESP32 USB troubleshooting](https://docs.espressif.com/projects/arduino-esp32/en/latest/troubleshooting.html),
  and the [ESP32-S3 USB Serial/JTAG guide](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32s3/api-guides/usb-serial-jtag-console.html).
- Espressif's [USB-Serial-JTAG driver notes](https://docs.espressif.com/projects/esp-iot-solution/en/latest/usb/usb_overview/usb_serial_jtag.html)
  for Windows and Linux hosts.

## What we confirmed (not assumed)

### Project choices already recorded

- M00 uses the N16R8 bench development board. Native USB is the selected programming and console
  connection in W01; the CH343P UART connector is not the selected path. GPIO19 and GPIO20 remain
  reserved for native USB. The product-reference evidence supports a YD-style carrier candidate,
  but does not identify or electrically validate the received carrier revision.
- The module designation used for the selected controller is ESP32-S3-WROOM-1-N16R8. Espressif's
  datasheet lists this module as 16 MB Quad-SPI flash and 8 MB Octal-SPI PSRAM. This is the working
  module configuration from the listing/project record, not a new inspection of the received
  board.
- The PlatformIO project belongs in `firmware/`. VS Code with the PlatformIO extension is the
  guided workflow; native Windows and native Linux are the intended M00 verification environments.
  WSL USB passthrough is outside the acceptance scope.
- The first program is limited to startup reporting and a low-rate periodic heartbeat. The loop
  should remain responsive, should not wait indefinitely for the monitor, and should run with no
  host attached. M00 does not require a unit-test framework; meaningful automated tests are to be
  considered with M01.

### PlatformIO and host setup

- PlatformIO's official VS Code extension supports Windows and Linux. The extension includes
  PlatformIO Core, so a separate system-wide Core installation is not required for the guided
  path. Its integrated terminal can run PlatformIO CLI commands. PlatformIO also notes that Linux
  installations need `python3-venv` for a smooth extension setup.
- PlatformIO documents its standard project configuration (`platformio.ini`) and source directory
  (`src/`), with `include/`, `lib/`, and `test/` available for project headers, local libraries,
  and tests. The repository can start with only the pieces M00 actually uses.
- PlatformIO recommends pinning the development platform in `platformio.ini` for repeatable
  checkouts. The project has selected the official `espressif32@8.0.0` release for M00; the
  resulting build and upload behavior still needs cross-platform validation.
- PlatformIO's general serial-access instructions mention the board manufacturer's Windows USB
  driver and Linux udev rules. Espressif separately states that the ESP32-S3's native USB
  Serial/JTAG interface needs no manually installed driver on Linux and that Windows 10+ generally
  installs its driver automatically when online. The actual host behavior must be checked with
  this board and the selected port; these general instructions do not prove this carrier's
  enumeration or upload path.

### Board profile and USB configuration findings

- PlatformIO's built-in `esp32-s3-devkitc-1` entry is explicitly for the N8 variant: 8 MB flash and
  no PSRAM. Its v8.0.0 manifest also defaults to an 8 MB partition layout. It must not be treated
  as an exact description of the project's N16R8 module without configuration overrides.
- The v8.0.0 manifest sets `ARDUINO_USB_MODE=1`, which selects the hardware USB-Serial/JTAG path in
  the Arduino-ESP32 core. Espressif's Arduino documentation explains that enabling
  `ARDUINO_USB_CDC_ON_BOOT=1` makes `Serial` use the USB CDC/JTAG interface in this mode. This is
  consistent with W01's native USB choice, but must be compiled and tested with the selected
  PlatformIO platform before becoming the project configuration.
- Espressif documents the USB Serial/JTAG interface as supporting both firmware flashing and a
  serial console. It normally appears as `/dev/ttyACM*` on Linux and a `COM*` port on Windows.
  These are expected naming patterns, not a fixed port assignment for the project.
- Espressif notes a one-time 50 ms wait can occur if the USB Serial/JTAG transmit buffer fills
  while no terminal is attached. That is bounded driver behavior, distinct from waiting
  indefinitely for a monitor. A short, low-rate heartbeat avoids relying on a connected host and
  minimizes unnecessary output.

### Toolchain release choice

As checked on 2026-10-23, PlatformIO's newest official Espressif 32 release is **8.0.0**, released
on 2026-10-19. Its release notes specify Arduino-ESP32 **3.3.12** (based on ESP-IDF 5.5.5) and a
newer toolchain. The preceding **7.1.x** release line uses Arduino-ESP32 **2.0.17** (based on
ESP-IDF 4.4.7); 7.1.3 was released on 2026-09-11 with a build-flag fix. PlatformIO documents
pinning an exact platform version, rather than leaving `platform = espressif32` unpinned.

**Decision:** Use the official `espressif32@8.0.0` release for M00 and pin that exact platform
version in the project configuration. PlatformIO supplies Arduino-ESP32 3.3.12 through this
platform release. The choice follows the current official release and does not rely on unverified
claims of compatibility: M00's Windows/Linux build and USB checks will establish whether it works
for this project. Reassess only if those checks or M01 dependency research reveal a concrete
incompatibility.

## Verification still required

- Build the candidate N16R8 profile and confirm its flash/PSRAM settings represent the listed
  WROOM-1-N16R8 module while using the project's accepted carrier-family assumption.
- Verify native USB upload and serial output from VS Code/PlatformIO on native Windows and native
  Linux. Record the tested OS releases and any actual driver/permission steps; do not prescribe
  generic driver changes unless the selected port requires them.
- Verify the heartbeat appears when monitoring and that the program keeps running with the monitor
  closed or no USB host connected.
- Distinguish all toolchain and firmware checks from W01's separate continuity, power-rail,
  display, ADC, sensor, and vehicle validation. No firmware build or physical test has been run as
  part of this research document.
