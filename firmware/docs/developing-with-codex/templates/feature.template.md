# Feature: <name>

A skeleton for the first of the five documents in a milestone's lifecycle. Fill in each
section, delete these italic notes, and expect the real document to grow past this outline —
this is a starting point, not a form.

## What is this feature?

*Plain language, no tech choices yet. Describe what the milestone does from the point of view
of the person or system that benefits from it, not how it will be built.*

> Example: "The gauge presents a live crankcase pressure reading and keeps the highest and
> lowest observed values available for review."

## Why it matters

*The problem this solves, or the risk it removes, in a sentence or two. Tie it back to the
larger project goal so a future reader understands why this milestone exists at all.*

> Example: "A repeatable live reading gives the driver a way to compare current crankcase
> behavior with measurements from similar operating conditions."

## What "done" looks like

*A short, concrete list of observable outcomes — not implementation tasks (that's what
`tasks.md` is for). Someone should be able to read this and know whether the feature shipped.*

> Example:
> - The gauge shows the current signed pressure with `inH2O` units.
> - Peak values update as new readings arrive and remain visible until deliberately reset.
> - An invalid reading is identified as invalid rather than displayed as a plausible pressure.

## What this feature is deliberately not

*Scope boundaries — what's tempting to fold in here but belongs in a later milestone instead.
This section prevents scope creep and gives the next milestone a clear starting line.*

> Example: "This does not establish sensor calibration or vehicle alarm thresholds. Those require
> separate research and measurement milestones."
