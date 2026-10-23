# Research: <name>

A skeleton for the second of the five documents in a milestone's lifecycle. Its job is to
separate what's actually verified from what's still assumed, and to surface open questions
before design work (`architecture.md`) begins.

## Prior art reviewed

*What you looked at before designing anything — existing code in this repo, similar features
elsewhere, relevant vendor docs or APIs, other teams' approaches. Cite specifics (file paths,
API endpoints, doc links), not vague impressions.*

> Example: "Reviewed the ADS1115 datasheet, the project ADC component guide, W01's harness
> source and the selected controller's proposed I2C pin allocation."

## What we confirmed (not assumed)

*Facts you verified directly — by reading real code, calling a real API, or running a real
command — rather than facts you're inferring or hoping are true. Each item should be something
you could point to as evidence if challenged.*

> Example: "The selected chain routes the FTP sensor output through the 470 ohm / 1 uF RC filter
> to ADS1115 A0; W01 records the intended electrical endpoints. This is a design fact, not a
> measured filter response or calibrated pressure reading."

## Open questions (for the human to answer)

*Anything you couldn't verify yourself, or any decision that depends on team context rather
than technical fact. Write these as direct questions, not as open-ended musing — the human
answers these before `architecture.md` is written.*

> Example: "Which measurement rates are useful for peak capture without overloading the display
> update path? Which behavior can be verified with the currently assembled bench hardware?"
