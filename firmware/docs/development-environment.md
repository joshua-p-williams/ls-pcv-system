# Firmware development environment

This guide sets up the firmware project in VS Code with PlatformIO and provides matching command
line instructions. The default `n16r8` build is now the M02 live sensor-to-display application on
the ESP32-S3 N16R8 development board. It reads the FTP sensor through the ADS1115, displays the
provisional family-model pressure on the GC9A01, and uses long press to cycle Gauge, Trend, and
Min/Max. The `n16r8_demo` target preserves the sensor-free M01 synthetic demonstration. The
`n16r8_adc` environment is a focused voltage-only bench utility that reports ADS1115 codes and
calculated A0 voltage without presenting an electrical test voltage as crankcase pressure. USB
serial output is optional diagnostics, not a heartbeat or runtime dependency.
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

The project has three environments: `n16r8` for the live application, `n16r8_demo` for the M01
synthetic demo, and `n16r8_adc` for M02 ADC-voltage bring-up. The two display environments use the
N16R8 board profile, which describes the ESP32-S3 WROOM-1
N16R8 memory configuration and enables the native USB CDC serial interface selected by W01. The
profile does not identify the exact carrier-board revision.

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

Use an explicit environment so the intended program is clear. `pio run -e n16r8` builds the live
M02 application; `pio run -e n16r8_demo` builds the M01 synthetic demo; `pio run -e n16r8_adc`
builds the M02 ADC bench utility. Upload commands use automatic port discovery when possible;
specify the board's actual port if needed:

```text
pio run -t upload --upload-port <serial-port>
```

Select the port shown by `pio device list` rather than committing a machine-specific port. The
native USB interface is expected to appear as a `COM` port on Windows and commonly as `/dev/ttyACM*`
on Linux. These names are examples only; confirm the actual device on the host.

## Build, upload, and observe the live M02 application

Build before connecting hardware:

```text
pio run
```

With the N16R8 board connected to the W01-selected native USB port, upload the firmware:

```text
pio run -t upload
```

Open the serial monitor at 115200 baud to see `M02_LIVE` acquisition diagnostics. The live gauge
uses the documented 1.500 V family zero default, so the recorded atmospheric sensor output may
appear as a small positive provisional value until the deliberate Settings zero workflow is
implemented. A long press cycles Gauge, Trend, and Min/Max; click and double-click do not change the
live measurement. Pressure is still provisional until Phase 5 bench characterization. The firmware
does not print a periodic heartbeat and does not wait for a monitor. The USB CDC transmit timeout
is zero so optional diagnostics cannot wait indefinitely for a host. Closing or reopening the
monitor must not reset the display or stop acquisition/button behavior.

To upload the preserved synthetic demo instead, select `n16r8_demo` in PlatformIO or run:

```text
pio run -e n16r8_demo -t upload
```

## Phase 3: verify ADC voltage acquisition

Build and upload the diagnostic-only environment:

```text
pio run -e n16r8_adc
pio run -e n16r8_adc -t upload
pio device monitor -p <serial-port> -b 115200
```

With the FTP sensor signal disconnected, the firmware reports an `M02_ADC` line containing
acquisition state, a changing sample sequence, acquisition time, raw ADC code, and calculated A0
microvolts. `state=voltage_read` means the ADC returned a code; it does not claim that the voltage
matches a pressure sensor's expected domain. A missing ADS1115 or bus problem appears as an
acquisition error. An unconnected A0 may float, so its displayed voltage is not a meaningful test
value. This firmware has no pressure display and labels pressure as not tested.

Before applying a battery, follow the ADS1115 and W01 pinout/wiring notes. With power disconnected,
isolate the FTP output from the A0-side test node so the battery cannot drive the sensor. Power the
ADC normally from the selected 5 V branch, then use the multimeter to confirm ADC VDD-to-GND and
measure the candidate battery voltage. Use only a source whose measured positive voltage is above
ADC ground and below the actual powered ADC VDD. Connect battery negative to the shared circuit
ground and battery positive to the isolated A0-side node; this node is `COND.OUT`/ADS1115 A0. The
battery is a signal source, not a board supply, and this direct-node check bypasses the series
resistor while retaining the node's shunt capacitor. Keep the circuit powered while A0 is connected,
and disconnect power before moving test leads. Compare the multimeter reading at A0 with `a0_uV`
converted to volts and note the raw code/sample sequence. Repeat with another measured safe source.
This checks acquisition direction and approximate voltage tracking; it does not calibrate the ADC,
RC response, or FTP sensor.

USB monitor output is optional and rate-limited. Closing the monitor should not stop ADC acquisition;
the current phase does not yet depend on the display or show pressure.

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
