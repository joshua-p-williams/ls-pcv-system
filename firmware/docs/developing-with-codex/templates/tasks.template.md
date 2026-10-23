# Tasks: <name>

A skeleton for the fifth of the five documents in a milestone's lifecycle — the concrete,
checkable list you'll actually track progress against during implementation. One `##` section
per phase from `plan.md`'s sequence, each with checkbox items; end with a wrap-up section.

## Phase 1: <short phase name, matching plan.md's sequence>

*Checkbox items concrete enough that "done" is unambiguous. Check them off (and add a short
deviation note) as you go, rather than rewriting history after the fact.*

> Example:
> - [ ] Add the approved board profile and named pin assignments.
> - [ ] Implement the smallest nonblocking controller bring-up behavior.
> - [ ] Record build and physical USB-console results independently.

## Phase 2: <short phase name>

*A new `##` section per phase — repeat this shape for as many phases as `plan.md` defined.*

> Example:
> - [ ] Initialize one approved peripheral from the board profile.
> - [ ] Verify it on the physical board and record any mismatch with the plan.

## Wrap-up

*The closing checklist every milestone ends with — the housekeeping that's easy to forget once
the "interesting" work is done.*

> Example:
> - [ ] All phase-specific build and verification steps are recorded accurately.
> - [ ] Any deviations discovered during implementation are reconciled back into `feature.md`,
>   `research.md`, `architecture.md`, or `plan.md` as appropriate.
> - [ ] Docs/README updated if this milestone changed how the tool is used.
> - [ ] Commit drafted for human review.
