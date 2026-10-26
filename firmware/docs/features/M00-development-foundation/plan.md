# Plan: M00 — Development Foundation

## Approach

Build the smallest useful PlatformIO project and its setup guide together so the guide can be used
to reproduce the same configuration on native Windows and Linux. Validate the project first with
builds, then verify upload and serial behavior on the N16R8 board. Update documentation from the
results, keeping software checks separate from electrical validation of the rest of the protoboard.

## Sequence

1. **Create the project scaffold and setup guide.** Add `platformio.ini`, a project-local N16R8
   board profile, and `src/main.cpp` with startup reporting and a low-rate, nonblocking heartbeat.
   Pin `espressif32@8.0.0` and configure the selected native USB CDC path. Write the first version
   of `firmware/docs/development-environment.md` for VS Code/PlatformIO and CLI workflows on
   native Windows and Linux. Update the firmware README and confirm the root README links to it.
   Build the project on both operating systems using the documented configuration; correct
   board-profile schema, memory, framework, or compiler issues before moving on.
2. **Verify upload and runtime behavior on the N16R8.** Follow the guide to upload through the
   native USB connection from both host operating systems. Confirm the startup message with the
   monitor open during reset and confirm periodic heartbeats continue afterward. Reset once with
   the monitor closed, then open it without intentionally resetting the board and check for the
   recurring heartbeat. Check the code for any wait on serial availability. If the monitor or USB
   connection resets the board on attachment, record that behavior and use the code path to
   establish that firmware startup does not depend on an open monitor. Record the actual
   board/host setup, observed port behavior, and any necessary OS-specific access steps.
3. **Reconcile the guide and close M00 verification.** Update the setup guide with observed
   PlatformIO/framework versions, tested OS releases, working build/upload/monitor steps, and
   actual USB access requirements. Clearly separate software verification from protoboard,
   peripheral, and vehicle checks. Confirm the firmware README and root README point to the right
   entry points; remove inaccurate setup assumptions and keep generated build files untracked.

## Verification / definition of done

- `pio run` completes on native Windows and native Linux from the same repository configuration,
  with `espressif32@8.0.0` and the N16R8 profile. The profile explicitly represents the listed
  16 MB flash / 8 MB Octal-SPI PSRAM module rather than silently inheriting the built-in N8
  defaults.
- `pio run -t upload` (or the matching PlatformIO VS Code upload action) successfully uploads to
  the N16R8 over its selected native USB connection on both host operating systems.
- Opening the serial monitor before reset shows a startup message followed by periodic heartbeats.
  Closing the monitor does not stop the loop, and the code contains no indefinite wait for serial
  availability. A later monitor session can observe recurring heartbeats.
- The setup guide gives working VS Code steps and equivalent CLI commands, identifies how to
  select the serial port without hard-coding a machine-specific port, and records only host access
  requirements observed during testing.
- The root and firmware README links resolve, the selected versions match the tested configuration,
  and no `.pio` build output or firmware binary is committed.
- No automated test framework is added solely for this scaffold. Build/upload/runtime results are
  recorded as such; no sensor, display, button, ADC, calibration, or vehicle behavior is claimed as
  validated.

## Risks / things to watch

- PlatformIO 8.0.0 is a new major release. If a concrete build or dependency incompatibility
  appears, record it and consult before changing the selected platform version.
- The built-in ESP32-S3-DevKitC-1 profile describes an N8 target. Verify the local board profile's
  schema and generated memory settings so the N16R8 flash and PSRAM assumptions are not lost to
  inherited defaults.
- The received carrier revision and its USB enumeration are not physically confirmed. Use the
  native USB path selected by W01, and report observed port behavior without generalizing it to
  every carrier revision.
- A startup line can be missed if the serial monitor opens after boot; the recurring heartbeat
  provides a later indication without requiring a reset. It remains diagnostic output and should
  not gate firmware execution.
- M00 has no non-serial application output. Running without a connected computer is supported by
  keeping serial optional and the main loop independent of it; that check does not demonstrate
  operation of any peripheral.
