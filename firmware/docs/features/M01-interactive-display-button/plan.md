# Plan: M01 — Interactive Display and Button

## Approach

Implement the synthetic gauge as one default PlatformIO firmware application, with a small
hardware-independent application core and thin N16R8 button/display adapters. Work from core rules
outward: first make the data and interaction behavior testable on the host, then connect the
physical button and display, and finally verify the integrated behavior on the bench. Keep the
LVGL/GC9A01 experiment as a reference; do not copy its application into the production firmware.

The plan follows the reviewed [feature](feature.md), [research](research.md), and
[architecture](architecture.md). It does not add sensor acquisition, calibration implementation,
production alarm limits, XIAO support, or settings persistence.

## Sequence

1. **Prepare pinned dependencies and input provenance.** Inspect the BetterButton implementation
   and choose the exact upstream revision to use. Record its source and license, then copy or adapt
   only the required library code into the planned private firmware library. Add the LVGL 9.2.2 and
   LovyanGFX 1.2.35 dependencies from ADR 0001 to the N16R8 environment. Preserve the existing M00
   build and heartbeat behavior while adding the new configuration. Compile a minimal integration
   before building application behavior; record any API or toolchain differences from the proof.
2. **Establish the host-testable gauge core.** Add focused types and modules for signed pressure,
   band classification, configurable demo bounds, deterministic synthetic value generation, the
   gauge session, one-second history, run extrema, and page/gesture state. Keep the core independent
   of Arduino, LVGL, LovyanGFX, and BetterButton. Configure PlatformIO Unity host tests as described
   in the architecture, and document the Windows GCC and Linux host-toolchain prerequisites and
   repeatable commands. Test threshold inclusivity, generator bounds and seeded behavior, time
   cadence, history fill/rollover, extrema, and gesture-to-state transitions before hardware work.
3. **Integrate the physical button and interaction loop.** Add the N16R8-specific input adapter using
   W01's GPIO6 proposal and the reviewed BetterButton API. Poll it on each nonblocking loop pass,
   translate emitted events to the temporary M01 mapping, and update the application session.
   Preserve the current generated value, history, and extrema when navigating pages. Build the
   N16R8 firmware and verify each physical gesture and diagnostics before adding display rendering.
4. **Build the three display pages.** Initialize the GC9A01 through LovyanGFX and LVGL using the
   proven N16R8 pin assignments. Implement the signed zero-referenced gauge with the `-14` to
   `+4.5 inH2O` synthetic span and the shared `-12, -8, -3, 0 inH2O` demo-band configuration. Add
   the 30-second trend and current-run min/max pages from the same session snapshot. Use LVGL's
   scale, chart, labels, and nonblocking animation/timer facilities; keep visual tuning to readable
   layout, signed ticks, status treatment, page identity, and demonstrated physical issues.
5. **Integrate and verify the complete N16R8 demonstration.** Confirm the normal PlatformIO build,
   upload, and monitor workflow. Check that startup and the main loop never wait for USB serial;
   button input, one-second history sampling, and LVGL service remain responsive together. Manually
   verify display initialization, all pages, status bands and boundaries, value changes, trend
   cadence/rollover, extrema retention, button gestures, and useful USB diagnostics. Record the
   actual board, display, wiring assumptions, and results; keep host-test results separate from
   physical verification.
6. **Reconcile the feature documentation and close M01.** Update this feature's plan/tasks and the
   firmware guide with actual dependency revisions, build/test/upload commands, host compiler setup,
   observed display behavior, and any deviations from the architecture. Update project status only
   for demonstrated outcomes. Keep the exploratory spike clearly identified as reference material
   and do not claim sensor, ADC, calibration, engine-threshold, or XIAO behavior.

## Verification / definition of done

- The default `n16r8` PlatformIO environment builds with the reviewed graphics stack and the
  BetterButton source revision documented; the M00 heartbeat is replaced by the agreed M01
  application without introducing another production firmware project.
- Hardware-independent application rules compile and pass PlatformIO Unity host tests on Windows
  and Linux using the documented host toolchains. Tests cover classifier boundaries, synthetic
  values within the configured visual span, deterministic generation, one-second sample timing,
  the 30-slot rolling history, run extrema, and temporary gesture/page behavior.
- On the N16R8, the physical GC9A01 displays readable gauge, trend, and min/max pages, and the
  physical button changes demo values and cycles pages as specified. Display refresh, gesture
  polling, sample history, and optional USB diagnostics operate together without blocking waits.
- The M01 gauge uses the family-reference span only for the synthetic demonstration. The `-12,
  -8, -3, 0 inH2O` bands remain explicitly demo-only. No numeric pressure is extrapolated by the
  future sensor path beyond the installed sensor's calibration-defined domain; out-of-range,
  electrical fault, and pressure-band status remain distinct concepts.
- Documentation records exact library provenance, reproducible setup and commands, host-test
  results, on-device checks, and any unverified behavior. No sensor/ADC acquisition, calibrated
  measurement, production warning threshold, XIAO support, persistence, or vehicle function is
  claimed as complete.

## Risks / things to watch

- **BetterButton integration:** permission is settled, but the implementation and selected upstream
  commit still need review. Preserve attribution/license and verify API, compile compatibility,
  timing, and behavior on the physical switch.
- **Native test toolchain:** Unity's native environment requires host GCC. Keep Windows and Linux
  setup instructions distinct where installation differs, while keeping the same core tests and
  source tree.
- **Target/display dependencies:** the graphics stack worked in the spike, but adding it to the
  default project may reveal PlatformIO dependency or configuration differences. Reuse the known
  initialization pattern without copying the experiment's full sketch or changing the tested
  library versions without documenting why.
- **Physical verification:** W01 is the connectivity authority, and the GPIO6 button mapping is a
  proposal. Software success does not prove wiring, power, display readability, color appearance,
  or switch behavior. Record actual bench observations separately.
- **Range interpretation:** the published `-14 to +4.5 inH2O` family reference points do not
  establish the actual aftermarket sensor's calibrated domain or saturation behavior. M01 uses
  generated values only; its scale and demo bands must not be described as live sensor limits or
  engine alarm settings.
- **UI scope:** prioritize readable output and the three agreed pages. Do not add DMA, double
  buffering, a UI designer, settings menus, or performance tuning unless an observed M01 problem
  demonstrates the need.
