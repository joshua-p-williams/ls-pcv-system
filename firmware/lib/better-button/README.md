# BetterButton

This private PlatformIO library contains the project-authored BetterButton gesture recognizer used
by M01. It reads a normally open switch between a GPIO and ground, configures the GPIO's internal
pull-up in `begin()`, and reports click, double-click, or long-press event codes from nonblocking
`input()` calls.

## Source and license

- Upstream: [joshua-p-williams/BetterButton](https://github.com/joshua-p-williams/BetterButton)
- Imported revision: [`e3a6bbaa6f72890d9dbb0105a754acc89c374ea3`](https://github.com/joshua-p-williams/BetterButton/tree/e3a6bbaa6f72890d9dbb0105a754acc89c374ea3)
- License: MIT; the upstream license is retained in this folder.
- `src/BetterButton.cpp` and `src/BetterButton.h` are copied unchanged from that revision.

## API and timing notes

Construct `BetterButton` with the selected GPIO, call `begin()` during hardware setup, then call
`input()` frequently from the main loop and handle each nonzero event once. The upstream defaults
are 60 ms debounce, 500 ms double-click monitoring, and 2000 ms long-press detection. They are
library defaults, not final gauge interaction requirements.

In this revision, the 500 ms double-click monitoring interval is measured from the first press
timestamp, not from the first release. The usable gap between clicks therefore depends on the
duration of the first press. Preserve the upstream code for provenance and verify that behavior on
the physical button during M01 integration before deciding whether a local adaptation is warranted.
Timing overrides, if needed, must remain compatible with the library's compile-time constants.

M01 uses the upstream gesture events only to drive synthetic demo behavior. The production button
mapping is deferred to later firmware work. The library does not establish switch wiring quality,
electrical noise immunity, or correct gesture behavior under the integrated display load; those
require bench verification.
