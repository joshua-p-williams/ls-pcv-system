# Firmware development environment

This guide sets up the firmware project in VS Code with PlatformIO and provides matching command
line instructions. The current firmware runs the M01 synthetic gauge demonstration on the ESP32-S3
N16R8 development board: it reads the multifunction button, draws three GC9A01 pages, and reports
gesture/state events over USB serial when a monitor is listening. The display and button operate
without a computer; serial output is optional diagnostics, not a heartbeat or runtime dependency.
Native Windows and native Linux are supported development environments; WSL does not count as the
native Linux verification environment.

The project pins PlatformIO Espressif 32 **8.0.0**, which supplies Arduino-ESP32 **3.3.12**. The
N16R8 firmware has built and uploaded from Windows using PlatformIO Core 6.2.0. The same N16R8,
button, and display configuration has been exercised on the bench. Native Linux host tests, build,
and upload remain pending; treat host-specific Linux steps below as instructions to verify.

## Prerequisites

- Install [Visual Studio Code](https://code.visualstudio.com/).
- Install the official [PlatformIO IDE extension for VS Code](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
  It includes PlatformIO Core for the guided workflow; a separate system-wide Core installation
  is not required.
- On native Linux, install `python3-venv` if it is not already available. PlatformIO's extension
  uses a Python virtual environment during setup.
- Connect the N16R8 development board through its W01-selected native USB port when uploading or
  using the serial monitor. Windows COM3 was observed on the development computer, but the port
  must be selected from the current host's device list. The received carrier revision is not
  identified by the firmware build profile.

## Open the project in VS Code

1. Open the repository's `firmware/` directory as the VS Code folder. This is the PlatformIO
   project root because it contains `platformio.ini`.
2. Allow the PlatformIO extension to initialize its project environment and download the pinned
   platform and framework packages. The first setup requires network access and can take several
   minutes.
3. Use the PlatformIO toolbar or Project Tasks to run **Build**, **Upload**, and **Monitor**.
   The configured serial monitor speed is 115200 baud.

The project currently has one environment, `n16r8`. The board profile describes the ESP32-S3
WROOM-1 N16R8 memory configuration and enables the native USB CDC serial interface selected by
W01. It does not identify the exact carrier-board revision.

## Use the command line

Open a terminal in `firmware/`. The PlatformIO IDE extension provides an integrated terminal with
its Core command available. A separately installed PlatformIO Core can also run these commands.

```text
pio --version
pio run
pio run -t upload
pio device list
pio device monitor -p <serial-port> -b 115200
```

`pio run` builds the default `n16r8` environment. The upload command uses automatic port
discovery when possible; specify the board's actual port if needed:

```text
pio run -t upload --upload-port <serial-port>
```

Select the port shown by `pio device list` rather than committing a machine-specific port. The
native USB interface is expected to appear as a `COM` port on Windows and commonly as `/dev/ttyACM*`
on Linux. These names are examples only; confirm the actual device on the host.

## Build, upload, and observe M01

Build before connecting hardware:

```text
pio run
```

With the N16R8 board connected to the W01-selected native USB port, upload the firmware:

```text
pio run -t upload
```

Open the serial monitor at 115200 baud to see startup information and gesture/state diagnostics.
The firmware does not print a periodic heartbeat and does not wait for the monitor to open. A
gesture changes the synthetic reading or page; the trend history samples once per second. The USB
CDC transmit timeout is set to zero so optional diagnostic output cannot wait indefinitely for a
host. Closing or reopening the monitor should not reset the display or stop button/display behavior.

## Host-specific USB notes

On the tested Windows setup, the native USB connection appeared as **COM3**. PlatformIO detected
the port, but its initial automatic connection attempt received no response. Holding **BOOT** while
pressing and releasing **RESET/EN** put the board into download mode; the upload then succeeded.
Record this as the observed procedure for this board, rather than assuming every carrier revision
needs it. On Linux, the port may be `/dev/ttyACM*`; access can depend on the distribution's serial
device permissions. Windows 10 and later generally install the native ESP32-S3 USB Serial/JTAG
driver automatically when online. Do not install a different driver or change Linux permissions
unless the selected device actually requires it.

## Initial M00 Windows setup observations

- **Windows build:** Passed on Windows 11 Pro, OS build 26200, with PlatformIO Core 6.2.0. The
  command `platformio.exe run -v` completed from `firmware/` using Espressif 32 8.0.0 and
  Arduino-ESP32 3.3.12. PlatformIO recognized the project-local N16R8 profile and reported 16 MB
  flash. The generated link used the `qio_opi` framework libraries and included the PSRAM
  initializer; the 8 MB capacity is specified by the selected N16R8 module, not measured by this
  compile.
- **Windows upload/serial:** The M00 VS Code PlatformIO upload task detected **COM3** on the
  W01-selected native USB connection. Automatic connection failed with “No serial data received.”
  Holding BOOT while pressing and releasing RESET/EN entered download mode, after which upload
  succeeded. M00's recurring heartbeat was observed at 115200 baud. Closing the monitor produced a
  Windows USB disconnect/reconnect notification; reopening the monitor restored heartbeat output.
  This is historical M00 behavior. M01 removes the heartbeat and has since been verified not to
  reset or stall when the monitor opens or closes. The upload log did not capture the PlatformIO
  Core/extension version.
- **Native Linux build:** Pending.
- **Native Linux VS Code setup, upload, and serial checks:** Pending.

## M01 verification record

The N16R8 build uses Espressif 32 8.0.0, Arduino-ESP32 3.3.12, LVGL 9.2.2, and LovyanGFX 1.2.35.
On 2026-11-01, `platformio.exe run -e n16r8` passed after the gauge scale labels were changed to
whole-unit increments. The physical display review confirmed the labels from `-14` through `+4`
were readable; the visible scale still extends to `+4.5 inH2O` without labeling that half-unit
endpoint.

Physical N16R8 testing also confirmed all three pages and the complete long-press page cycle,
normal/warning/danger demo color agreement, more than 50 consecutive registered clicks after
button polling moved to a FreeRTOS task, one-second trend sampling, partial startup history,
30-sample rollover, retained min/max values, and responsive operation with the serial monitor open
or closed. These checks cover synthetic UI behavior only; they do not verify the FTP sensor, ADC,
calibration, vehicle operation, or production alarm thresholds.

A successful firmware build or USB serial session does not verify the protoboard's peripheral
wiring or electrical behavior.

## Native host tests

The `native` PlatformIO environment runs hardware-independent C++ tests on the development
computer. It builds the `gauge-core` private library and Unity tests without compiling
`src/main.cpp`, Arduino, LVGL, LovyanGFX, or connecting a controller. This checks shared pressure
conversion, validity and status boundaries, trend gaps/extrema, zero-capture evaluation, synthetic
value generation, and page behavior; it does not verify ESP32 integration or physical hardware.

PlatformIO's native environment uses the host's GCC toolchain rather than installing one itself.
On Windows, install the toolchain as follows:

1. Download and install MSYS2 from the [official MSYS2 site](https://www.msys2.org/).
2. Launch the UCRT64 shell from PowerShell. This explicit command avoids accidentally opening
   another MSYS2 environment such as MINGW64:

   ```powershell
   & 'C:\msys64\msys2_shell.cmd' -defterm -here -no-start -ucrt64
   ```

   In the new shell, `echo $MSYSTEM` should print `UCRT64` (not `MINGW64`), and `which pacman`
   should locate `/usr/bin/pacman`. Install GCC, which includes both the C and C++ compilers:

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```

3. Add the UCRT64 binary directory to the Windows `PATH`. For the default MSYS2 installation this
   is `C:\msys64\ucrt64\bin`.
4. Open a new VS Code terminal and verify that both compilers are available:

   ```powershell
   gcc --version
   g++ --version
   ```

On Debian or Ubuntu Linux, install the host compiler with `sudo apt update` followed by
`sudo apt install build-essential`, then verify `gcc --version` and `g++ --version`. Install
PlatformIO through the VS Code extension as above, or use the existing PlatformIO Core CLI.

From `firmware/`, run the appropriate PlatformIO command for the host:

```text
# Windows (PlatformIO IDE terminal)
platformio.exe test -e native

# Linux
pio test -e native
```

In VS Code, use the integrated terminal opened in `firmware/`; PlatformIO's test tasks can also
select the `native` environment. The command reports each Unity test and exits unsuccessfully if a
test fails. The `n16r8` environment remains the normal firmware build and upload target; the native
test environment is not a substitute for `pio run -e n16r8` or physical verification.

PlatformIO's official [Native platform guide](https://docs.platformio.org/en/latest/platforms/native.html)
documents host compiler prerequisites, the [mingw-w64 MSYS2 guide](https://www.mingw-w64.org/getting-started/msys2/)
documents the Windows package installation, and the [Unity framework guide](https://docs.platformio.org/en/latest/advanced/unit-testing/frameworks/unity.html)
documents native test support.

### M01 host-test verification status

**Windows passed:** Windows 11 Pro, MSYS2 UCRT64 GCC/G++ 16.2.0, PlatformIO Core 6.2.0, native
platform 1.2.1, Unity 2.6.1. `platformio.exe test -e native` ran from `firmware/` in the UCRT64
shell using the PlatformIO Core executable installed in the user profile; all 12 tests passed.
Native Linux has not yet been tested; use the same test sources and `pio test -e native` there.
