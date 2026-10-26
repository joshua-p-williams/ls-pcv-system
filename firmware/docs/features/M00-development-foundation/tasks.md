# Tasks: M00 — Development Foundation

## Phase 1: Project scaffold and setup guide

- [x] Create the PlatformIO project at `firmware/` with `platformio.ini`, `boards/`, and `src/`;
  do not add a nested project-name directory.
- [x] Add the project-local N16R8 board profile. PlatformIO 8.0.0 accepts its schema; the build
  reports 16 MB flash and uses `qio_opi` libraries with PSRAM initialization enabled. The 8 MB
  PSRAM capacity is based on the selected module designation, not measured by compilation.
- [x] Configure the Arduino framework, exact `espressif32@8.0.0` platform pin, N16R8 profile,
  and native USB CDC settings in `platformio.ini`.
- [x] Implement a startup message and low-rate heartbeat in `src/main.cpp`. Use a nonblocking
  elapsed-time check and do not wait for serial availability or initialize peripherals.
- [x] Create `firmware/docs/development-environment.md` with prerequisites and setup steps for
  native Windows and Linux, VS Code/PlatformIO extension workflow, equivalent CLI commands,
  serial-port selection, and build/upload/monitor instructions. Label steps not yet exercised as
  unverified until Phase 2.
- [x] Update `firmware/README.md` with a concise getting-started entry and link to the setup guide;
  confirm the root README links to the firmware entry point.
- [x] Ensure PlatformIO's local build output is ignored narrowly (for example, `.pio/`) without
  ignoring source-controlled firmware or engineering artifacts. The existing root `.gitignore`
  already contains `/firmware/.pio/`.
- [x] Run the project build on native Windows using the same project configuration. PlatformIO
  Core 6.2.0, Espressif 32 8.0.0, and Arduino-ESP32 3.3.12 completed successfully on Windows 11
  Pro (build 26200).
- [ ] Run `pio run` on native Linux using the same project configuration. The native Linux machine
  is not available in this session; record its OS and PlatformIO versions and results when run.

## Phase 2: N16R8 upload and runtime verification

- [x] Use the documented VS Code PlatformIO workflow on Windows 11 Pro (OS build 26200), record
  the resolved Arduino framework version (3.3.12) and COM3, and identify the missing version data:
  the upload log did not capture the PlatformIO extension/Core version.
- [ ] Complete the documented setup on native Linux and record its OS release, PlatformIO
  extension/Core version, resolved framework version, and actual upload/monitor port.
- [x] On Windows, upload the firmware to the N16R8 over the W01-selected native USB connection;
  PlatformIO detected COM3. Automatic connection did not succeed, so the board was manually put
  into download mode with BOOT held while RESET/EN was pressed and released.
- [ ] Repeat the upload on native Linux using the W01-selected native USB connection.
- [x] Confirm recurring heartbeat output on Windows at the configured 115200 baud. The one-time
  startup print is present in `setup()`, but was not captured during USB reset/reconnect; this is
  consistent with the intentional no-wait-for-serial behavior.
- [x] Reset with the monitor closed, then attach it without intentionally resetting again; the
  heartbeat was visible. Windows reported USB disconnect/reconnect at reset. The startup and main
  loop do not wait for the monitor to be available.
- [x] Confirm the firmware has no monitor-dependent code path by source review and by observing
  heartbeat after booting with the monitor closed. This does not validate peripherals or the
  vehicle installation.
- [ ] Record native Linux USB enumeration, upload, and any OS-specific access steps. Do not
  prescribe driver or permission changes that were not needed or observed.

## Phase 3: Documentation reconciliation and M00 closeout

- [x] Record the observed Windows build, upload, COM3 enumeration, manual download-mode sequence,
  USB reset/reconnect behavior, heartbeat, and the uncaptured one-time startup line in the setup
  guide; keep Linux checks explicitly pending.
- [ ] Complete the setup guide's verification record with native Linux versions, commands, USB
  behavior, and any access steps actually observed.
- [x] Recheck that the firmware README and root README links resolve and the documented versions
  match the tested `platformio.ini` configuration.
- [x] Confirm no generated `.pio` output, firmware binaries, or machine-specific port settings are
  included in the public project files. The `.pio` output is ignored and untracked; COM3 appears
  only as a recorded Windows observation, not as a configured upload port.
- [x] Review the firmware changes and ensure claims distinguish software build/upload/serial
  checks from protoboard continuity, peripheral operation, calibration, and vehicle validation.

## Wrap-up

- [x] Record the Windows phase results and deviations in the relevant M00 documents; the native
  Linux work remains open.
- [x] Run available documentation/link checks and `git diff --check`; root and firmware README
  links/anchors passed. The repository checker parsed JSON/SVG files but could not finish its
  Markdown/W01 report because WireViz is not installed in the temporary Python environment.
- [x] Review `git status` and the complete diff, including new files. Leave staging and commit
  creation to the repository's normal human review process.
