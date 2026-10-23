# Architecture: <name>

A skeleton for the third of the five documents in a milestone's lifecycle. Written only after
`research.md`'s open questions are answered — this is where design decisions get made and
recorded, not where new questions get asked.

## Overview

*A short paragraph or two summarizing the shape of the solution — what it is, at a level
someone could repeat back after one read. Save the details for the sections below.*

> Example: "A firmware acquisition path reads the ADS1115 independently from display drawing,
> converts valid samples using an explicit calibration, and publishes a signed pressure value
> for the display and peak tracker."

## What gets built & how it fits together

*The concrete pieces — new classes/modules/files, how they call each other, and how this
milestone's output connects to what already exists or to what a later milestone will consume.*

> Example: "A board profile supplies named GPIO assignments; an ADC adapter returns raw
> conversion results; a measurement layer owns calibration and validity; the display consumes a
> shared gauge state. These boundaries keep board-specific wiring out of pressure math."

## Key decisions and why

*Each real decision point, the option chosen, and the reasoning — especially anywhere you
picked one approach over an equally plausible alternative. This is the section future readers
will thank you for; it answers "why is it built this way?" without them having to ask you.*

> Example: "Keep measurement acquisition independent of display refresh so a slow screen update
> cannot cause the peak tracker to miss a pressure sample."

## What's explicitly out of scope

*Anything adjacent that this design deliberately does not attempt to solve, so a future reader
doesn't mistake an omission for an oversight.*

> Example: "Vehicle alarm thresholds are outside a bench acquisition milestone; no threshold is
> supported until the system has been calibrated and its behavior characterized."
