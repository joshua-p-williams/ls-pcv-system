# Firmware Coding Standards

These standards make firmware easier to understand, verify, and extend. The source is also an
instructional resource for readers who may be new to Arduino, FreeRTOS, LVGL, or embedded C++.
When choosing between a useful explanation and an unexplained implementation detail, favor the
explanation. Comments should clarify design and concepts without merely narrating obvious syntax.
Feature architecture documents define the concrete modules and interfaces for each milestone.

## Explain the code

- Give every function and method a concise Doxygen-style summary, including private helpers and
  test functions. State what it does and why it exists; document parameters, units, return meaning,
  side effects, preconditions, and failure behavior where relevant.
- Add comments inside nontrivial algorithms to explain concepts, timing, ownership, invariants,
  constraints, and reasons for a choice. Assume the reader may understand C++ but not the specific
  Arduino, ESP32, FreeRTOS, LVGL, or driver-library API being used.
- Explain a library's role at its integration boundary: what it provides, what the project adapter
  does, and which task/context is allowed to call it. Link to the component README or feature
  architecture for longer explanations instead of duplicating large reference sections inline.
- Keep comments beside the behavior they explain. Mark synthetic/demo behavior, provisional values,
  hardware assumptions, and replacement points explicitly so they cannot be mistaken for measured
  behavior or production rules.
- Use names that carry meaning, especially for pressure, voltage, time, and calibrated values.
  Include units in identifiers when that prevents a unit conversion from being hidden.
- Keep instructional documentation near its code or subsystem. Explain how the implementation
  relates to the hardware and relevant feature/architecture documents without copying long pin maps
  or specifications into source comments.

## Design clear responsibilities

- Keep board-specific pins, peripheral setup, and hardware drivers separate from pressure
  classification, demo behavior, and other application rules.
- Give each module a cohesive responsibility. Keep dependencies pointed toward small, stable
  contracts rather than letting UI code own measurement rules or hardware details.
- Apply SOLID as a set of design checks: prefer single responsibilities; extend behavior through
  explicit seams where change is expected; substitute implementations only when they honor the
  same contract; keep interfaces focused; and depend on abstractions at real hardware or behavior
  boundaries.
- Use abstractions where they make a likely change or a test boundary simpler—for example, between
  a pressure source and the gauge logic, or between an input device and gesture handling. Do not
  create interfaces, service layers, or generic frameworks solely to satisfy a pattern. The feature
  architecture document chooses the actual layers and contracts.
- Keep hardware-dependent behavior explicit. A successful host build or unit test does not verify
  wiring, voltage compatibility, timing, or physical display behavior.

## Test behavior

- Add automated tests for meaningful logic, especially boundary conditions, invalid input, units and
  sign handling, state transitions, and deterministic behavior.
- Make synthetic sources and time-dependent logic reproducible in tests. Avoid tests that depend on
  unseeded randomness, real delays, or attached hardware when the behavior can be tested separately.
- Keep hardware verification distinct from unit tests. Record the board, configuration, and actual
  result for manual bring-up checks.
- Document reproducible build and test commands in the relevant firmware guide. Run the applicable
  checks after changing behavior and report checks that could not be run.
- Prefer tests that verify observable behavior and important boundaries; do not add tests that only
  mirror private implementation details or provide no meaningful regression protection.

## C++ and consistency

- Follow the existing firmware style: two-space indentation, braces on the declaration line,
  descriptive `lowerCamelCase` variables/functions, `UpperCamelCase` types, and `kUpperCamelCase`
  constants.
- Prefer explicit types, `const` where appropriate, clear ownership, and small functions. Avoid
  blocking waits in the main application path when they would delay button handling, display work,
  diagnostics, or future measurement acquisition.
- Keep formatting consistent with nearby code. Add a formatter or stricter static checks only when
  the project has a practical way to run them reproducibly.

## Record lasting decisions

Use an [architecture decision record](decisions/README.md) for a consequential choice with real
alternatives and future effects, such as selecting a graphics stack or a shared hardware boundary.
Keep rationale, considered alternatives, and consequences in the ADR. Routine implementation
details belong in code, feature architecture, or research documents; do not create an ADR for every
small choice.
