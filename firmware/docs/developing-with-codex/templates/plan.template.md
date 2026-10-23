# Plan: <name>

A skeleton for the fourth of the five documents in a milestone's lifecycle. Turns
`architecture.md`'s design into an ordered sequence of work with a way to tell when each step —
and the milestone as a whole — is actually done.

## Approach

*A short statement of how you'll tackle the work overall — the order of concerns, not yet a
step-by-step list. This sets context for the sequence below.*

> Example: "Confirm the selected board configuration and console path first, then bring up one
> peripheral at a time, recording the check that demonstrates each interface works."

## Sequence

*The ordered, numbered list of concrete steps or phases. Each step should be small enough to
build and verify on its own before moving to the next — this is what `tasks.md` will expand
into a checklist.*

> Example:
> 1. Build and flash a controller-only image and verify optional USB diagnostics.
> 2. Initialize the display and verify reset, SPI, orientation and readable output.
> 3. Read raw ADS1115 samples over translated I2C before adding pressure calibration.

## Verification / definition of done

*How you'll know each step worked, and how you'll know the whole milestone is done — specific
commands to run, tests to pass, manual checks to perform. Avoid vague statements like "make
sure it works"; say what "working" is checked against.*

> Example: "The documented firmware build succeeds; the assembled N16R8 starts with and without
> a connected serial monitor; a bench check confirms the selected display interface. Record
> build output separately from physical observations."

## Risks / things to watch

*Anything that could go sideways during implementation — flaky dependencies, ambiguous edge
cases, performance concerns — so it's flagged before it becomes a surprise mid-task.*

> Example: "Virtual Wokwi parts may not model the purchased ADC or translator electrically;
> distinguish software simulation from measurements on the assembled hardware."
