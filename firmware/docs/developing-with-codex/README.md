# Developing Firmware with Codex CLI: A Practical Workflow

This document adapts a specific way of working with an AI coding agent — Codex CLI — on a
real, ongoing, multi-feature firmware project: brainstorm before building,
write your standards down once, and work through every feature via five short documents before
any code gets written. It's a recipe that worked well for us on a project with real substance,
not *the* only correct way to work with an agent — treat it as a strong starting point, not a
law.

None of this is actually new. It's the same discipline software engineering has always taught —
understand the problem, design before you build, break the work into a plan, then implement
against that plan and verify it — just applied to working *with* an agent instead of alone.
Under the ordinary pressure of shipping, that discipline is often the first thing experienced
engineers quietly let slide, because doing it properly by hand is slow, and skill lets you get
away with skipping it more often than not. An agent removes that trade-off: it can produce a
thorough requirements doc, a design, and a task breakdown in minutes instead of days, so you
keep the rigor without paying its old cost in time — and that's precisely where the speed gain
actually comes from, not from skipping the thinking. The agent can write code quickly, but it
still needs you to do the thinking: to know what's actually being asked for, to judge whether a
design is sound, to catch the assumption that's quietly wrong. This workflow is mostly just a
structure for putting that thinking back where it belongs, and letting the machine do what it's
good at — which, done well, is less "reinventing how you work" and more "getting back to how
you were probably taught to work in the first place."

## Table of Contents

- [Quick Recipe](#quick-recipe)
- [Why This Works](#why-this-works)
- [Setup: Tools and Assets to Have on Hand](#setup-tools-and-assets-to-have-on-hand)
- [Phase 1: Brainstorm and Gather Context First](#phase-1-brainstorm-and-gather-context-first)
- [Phase 2: Establish Durable Standards Once, Up Front](#phase-2-establish-durable-standards-once-up-front)
- [Phase 3: The Five-Document Feature Lifecycle](#phase-3-the-five-document-feature-lifecycle)
- [Phase 4: Working Through Implementation](#phase-4-working-through-implementation)
- [Prompt Patterns for Future Firmware Work](#prompt-patterns-for-future-firmware-work)
- [The Kickoff Prompt](#the-kickoff-prompt)
- [When This Is (and Isn't) Worth It](#when-this-is-and-isnt-worth-it)
- [Further Reading](#further-reading)

## Quick Recipe

- **Set up**: open this repository with Codex CLI from its root, then gather the
  reference material you already have (standards docs, sample projects, a shared knowledge
  source).
- **Gather context**: brainstorm the problem in plain language before writing anything, and
  point the agent at real existing code/docs instead of describing conventions abstractly.
- **Establish standards**: capture naming/design conventions in durable markdown docs, and keep
  an agent-instructions file current that points into them.
- **Name your milestones**: write down a rough, ordered list of the independently-consumable
  pieces the project breaks into.
- **Run the five-document cycle per milestone**: `feature.md` → `research.md` →
  `architecture.md` → `plan.md` → `tasks.md`, one at a time, stopping for review after each.
- **Work `tasks.md` phase-by-phase**: implement only the phase you're told to, build and test
  it, then stop and wait.
- **Reconcile deviations immediately**: fold anything you learn during implementation back into
  the relevant document rather than leaving it undocumented.
- **Commit at checkpoints**: let a human author the commit once a phase or milestone is
  verified.

Examples in the templates are instructional examples only. They do not select a milestone,
electrical behavior, pin assignment, library or implementation detail for this project.

See the full [Guide](#why-this-works) below for the reasoning, setup detail, and a copy-paste
[kickoff prompt](#the-kickoff-prompt).

---

## Why This Works

A vague prompt forces an AI coding agent to silently guess at potentially thousands of unstated
requirements. It will make reasonable assumptions, some of them wrong, and you typically don't
discover which ones until you're deep into an implementation built on the wrong premise. The
fix isn't "write better prompts" so much as it is: separate *deciding what to build and why*
from *deciding how to build it* from *actually building it*, and put a human checkpoint between
each — on disk, not just in conversation, so the decisions survive past the point the
conversation's context window starts to fill up and earlier detail starts getting lost.

This isn't just our own preference. GitHub's open-source **Spec Kit** — a
spec-driven-development workflow for Codex CLI and other coding agents — describes
almost the same discipline: **Specify** (what and why) → **Plan** (how, technically) →
**Tasks** (small, independently reviewable chunks) → **Implement** (work the tasks, review
focused diffs), with an explicit rule that you don't move to the next phase until the current
one is fully validated. GitHub also flags that ongoing feature work in an existing codebase —
as opposed to a brand-new, single-shot project — benefits from *more* context engineering than
that base four-phase shape provides. The workflow below is, in effect, one answer to that gap:
it splits "Plan" into a design phase and a sequencing phase, and adds an explicit research phase
before any design commitment, so investigation, design, and sequencing don't blur together into
one pass. Treat that as our elaboration on the documented pattern, not a replacement for it —
lighter projects may not need all five documents (see
["When This Is (and Isn't) Worth It"](#when-this-is-and-isnt-worth-it)).

Anthropic's published **Claude Code best practices** reinforce the same shape from the
implementation side, and add the mechanical reason it matters: a model's context window fills
up, and performance degrades as it fills — it can start "forgetting" earlier instructions the
longer a session runs. Their guidance — explore before planning before coding, give the agent a
concrete way to verify its own work rather than letting "looks done" be the only signal, point
it at real existing patterns instead of describing them abstractly, and keep instructions files
short and concise — is a near-exact match for what's described here, arrived at independently.
See [Further Reading](#further-reading) for both sources directly; the rest of this guide
doesn't re-argue the case, it just shows the workflow in practice.

## Setup: Tools and Assets to Have on Hand

**The tool.** Run Codex CLI from the repository root so it can read the project instructions and
the files relevant to the current milestone. The root `AGENTS.md` is the durable entry point;
Codex recognizes these instruction files and applies more specific instructions from deeper
folders when present. Give the agent repository-relative paths to important documents or ask it
to inspect them directly. Access to sibling repositories or other locations depends on current
workspace permissions; provide or approve that access only when it is needed. An editor can be
open alongside the CLI, but editor integration is not a requirement of this workflow.

**Assets worth gathering before you start.** None of these are hard prerequisites — gather
whatever you actually have, and don't block on the ones you don't:

- **Existing coding standards, written down.** Even a rough draft is useful; the agent can help
  you refine one from real code if you don't have one yet (see
  [Phase 2](#phase-2-establish-durable-standards-once-up-front)).
- **Architecture Decision Records (ADRs)** or similar design-history documents, if your project
  already has them — they're excellent ground truth for "why do we do it this way."
- **Sample or reference projects** — a repository (or a folder of them) that already
  demonstrates the patterns you want new work to follow. Pointing an agent at a real, working
  example beats describing the pattern in prose almost every time.
- **A shared team knowledge source**, if you have one — an internal wiki, a shared GitHub
  project knowledge source, or similar — especially useful for anything that isn't captured in code or
  docs, like team-specific process knowledge.
- **Relevant MCP servers or tools** your organization has already configured (for source
  control platforms, issue trackers, internal APIs, etc.) — these extend what the agent can
  directly look up or act on, instead of you relaying information by hand.

## Phase 1: Brainstorm and Gather Context First

Before any file gets created, spend one or more sessions just talking. Explain the problem
domain in plain language: what the thing you're building is for, the vocabulary specific to
your team or industry, the constraints that aren't obvious from the code alone (approval
gates, ordering dependencies, naming conventions). Don't expect to have to volunteer everything
unprompted, either — a good agent session here looks like an interview, with the agent actively
asking follow-up questions to surface the details you didn't think to mention, not passively
waiting for a complete brief. Nothing gets written to disk yet — the goal is a shared
vocabulary, not an artifact.

This matters more than it sounds like it should. An agent that's asked to produce a design
before it understands the domain will produce a *plausible* design — one that could be right
for a generic version of your problem, but quietly wrong for your actual constraints. A
conversation-only phase up front is cheap insurance against that.

Once the vocabulary is established, **point the agent at real, existing examples on disk**
rather than describing your conventions abstractly. If your organization already has other
projects that follow the pattern you want — a similar service, a similar library, a similar
pipeline — have the agent search for and read them directly ("look at how these existing
projects under this path are structured"). An agent reading real, working code will infer your
actual conventions far more reliably than one working from a prose description, however
careful, because prose can be incomplete or subtly out of date in ways a real example can't be.

One more outcome of this initial brainstorm is worth naming explicitly, because it's what the
rest of this workflow builds on: talk through the whole project at a high level, and use that
conversation to identify **milestones** — a rough, ordered list of logical, independently-
consumable pieces the project breaks into (a bootstrap/scaffolding step first, then each
feature roughly in the order you'll tackle it). Say the word "milestones" out loud in this
brainstorm and ask the agent to help you name and order them; don't leave this list implicit in
your head. It doesn't need the formal five-document treatment described in
[Phase 3](#phase-3-the-five-document-feature-lifecycle), and it's expected to change as you
learn more — treat it as a working sketch, not a commitment. But *do* have the agent write it
down somewhere lightweight before the conversation moves on — a short note, a section at the
top of a planning doc, whatever's convenient. The same reasoning that makes the five documents
durable files instead of chat history applies here too: a sketch that exists only in
conversation is exactly the kind of thing a compaction event or a new session can quietly lose.
Naming the milestones explicitly, even loosely, and writing them down, turns "the project" from
one big undifferentiated thing into a sequence you can actually work through one item at a
time, using the process in [Phase 3](#phase-3-the-five-document-feature-lifecycle), below.

## Phase 2: Establish Durable Standards Once, Up Front

Before starting the first real feature, get your conventions written down as their own
standalone documents — not buried in a single instructions file, and not left as unwritten
tribal knowledge. Have the agent help extract them from real prior art: existing projects,
an internal style guide, prior design-decision records, whatever you already have. Cover the
things that would otherwise get silently reinvented per feature — firmware source layout, C++
naming and ownership conventions, board-profile boundaries, dependency policy, nonblocking
acquisition, units and numeric conversions, error reporting, documentation and verification.
Treat these as topics to discuss and document, not decisions to invent in advance. This is a
one-time, front-loaded cost; every
feature that comes after it references these docs instead of re-deriving the same conventions
from scratch, and disagreements about "how we do things here" get resolved once instead of
once per feature.

If you're organizing these into their own folders (a standards folder, an architecture-decision-
record folder, and so on), consider giving each one a short `README.md` explaining what it's
for and how it's meant to be used — this document's own project does exactly that for its
standards and feature folders. It's a small thing, but it turns "why does this folder exist"
into something the agent (and any new teammate) can just read instead of asking.

Then write (or update) the agent-instructions file. This repository uses its root `AGENTS.md`
as the canonical project guide. Codex recognizes `AGENTS.md` and reads applicable instructions
from the repository root down to the working directory. Keep this file focused on durable
project rules and pointers to the firmware workflow and hardware sources; do not maintain
duplicate agent-specific instruction files.

Keep it short. Its job is to identify the project, point to durable standards and define the
workflow gates. Put the full process in this guide and have `AGENTS.md` point here; do not
duplicate it in the instruction file. This keeps session-level instructions visible while the
detailed procedure remains readable and maintainable.

A practical shortcut: Codex can help draft an instruction-file summary from the project
documents. Review that draft and decide what belongs in durable instructions.

For this repository, the durable firmware conventions are collected in the
[firmware coding standards](../coding-standards.md), while the canonical project workflow remains
in `AGENTS.md` and this guide. The
[firmware decision records](../decisions/README.md) capture selected design trade-offs. Feature
architecture documents still define the actual layers and interfaces for each milestone; do not
try to prescribe those details in a cross-project coding standard.


It is useful to prove the process on a small first milestone. Its scope should exercise the
approved documents, implementation and verification without attempting the whole gauge at once.


## Phase 3: The Five-Document Feature Lifecycle

**This is not something you do once for the whole project — it's a repeatable cycle you run
once per milestone (a.k.a. feature).** After the initial brainstorm and standards setup, don't
try to design the entire system in one shot. Take the milestones you named in
[Phase 1](#phase-1-brainstorm-and-gather-context-first) — bootstrap first, then each logical,
independently-consumable feature in roughly the order you'll tackle it — and work through that
list one milestone at a time. Each one gets its own trip through the five documents below,
start to finish, before you move on to the next. Think of it less like "planning the project"
and more like a small, disciplined unit of work you apply repeatedly: pick the next
milestone/feature, produce the five documents for it, implement it, verify it, commit it, then
pick the next one. That repeatability — not any single document — is the actual core of this
workflow. Your milestone list itself isn't fixed either; it's fine to revise it, split a
milestone, or reorder what's next as you learn more from finishing each one.

Codex can help reason through a plan in conversation, but that plan is not a substitute for the
approved files below. This workflow is deliberately more durable because firmware milestones
span multiple sessions, depend on hardware evidence, and need decisions to remain reviewable.

For each feature, write these five documents in order, with a deliberate human checkpoint
between each one. The agent writes a document and stops; you review it and either approve it or
send it back with changes; only then does the agent move to the next document. Never let the
agent barrel through several of these unsupervised, even if it offers to — the checkpoints are
where you catch a wrong assumption while it's still cheap to fix, before any code exists.

It's worth being explicit about *why* these are separate, durable files rather than just a long
conversation, because it follows directly from how these agents actually work. An LLM's context
window — the text it can actually "see" at once — is finite, and a long-running session will
eventually fill it. When that happens, most tools compact the conversation: they summarize
everything so far to make room for more, which is useful, but summarization is lossy by
definition — a subtle decision made ten messages ago can quietly drop out. A brand-new session,
meanwhile, remembers nothing at all; it starts from zero. If the only record of a decision is
"somewhere earlier in this chat," it is one compaction event or one new session away from not
existing. A markdown file committed to your repository doesn't have that problem — it gets
re-read fresh every time it's needed, in full, regardless of how much conversation has happened
since it was written or who (human or agent) is picking the work back up. That's the real,
mechanical reason this workflow insists on writing decisions down as it goes, instead of
trusting that "we agreed on this earlier in the chat" will still be true later.

1. **`feature.md`** — what this feature is and why, in plain language, from the user's or the
   business's point of view. No technology choices yet. This is the "specify" step: get
   agreement on the problem before anyone starts solving it.
2. **`research.md`** — the investigation phase: what already exists that's relevant, what the
   real APIs/libraries/constraints actually look like (verified, not assumed), and — critically
   — an explicit list of open questions the agent genuinely cannot answer on its own. Don't let
   the agent guess at these; answer them yourself, directly, in the next message, often as an
   opinion ("I think X, because Y — what do you think?") rather than an open-ended question
   back. That gives the agent something concrete to refine instead of a blank page.
3. **`architecture.md`** — the design: what gets built, how the pieces fit together, and why,
   now that the facts from `research.md` are settled. This is where technical trade-offs get
   decided and written down, not just discussed and forgotten.
4. **`plan.md`** — the sequencing and verification approach: what order the work happens in,
   and how you'll know each part is actually done (what gets built, what gets tested, what gets
   manually verified). This turns the design into an executable strategy.
5. **`tasks.md`** — a concrete, checkable list derived from the plan. This is the actual unit of
   work-tracking for the feature — not a scratch, in-session todo list that disappears when the
   conversation ends, but a real file, committed to your repository, that survives a new
   session, a context-window compaction, or a different person picking up the work later.

One habit makes this whole cycle trustworthy over time: **when implementation reveals that an
earlier document was wrong or incomplete — a missing parameter, an incorrect assumption, a
pattern with no precedent in your codebase — fix it in two places, not one.** Correct the
running code, and also edit the document that was wrong, noting what changed and why (a
"deviation found" note in `tasks.md` is a simple, effective place for this). Skipping the second
half of that is how documentation quietly rots into fiction. Doing both, every time, is why
these documents are still worth trusting after a project has gone through many rounds of this
cycle.

This maps closely onto — and deliberately extends — the specify → plan → tasks → implement
shape described in [Why This Works](#why-this-works): the same underlying discipline, split
into more phases because ongoing, multi-feature work on an existing codebase benefits from
separating "what did we learn" (`research.md`) from "what did we decide" (`architecture.md`)
from "how do we sequence and verify it" (`plan.md`), rather than collapsing all three into a
single planning pass.

## Phase 4: Working Through Implementation

Once `tasks.md` exists for a milestone, work it one phase or section at a time, explicitly
chosen by you ("let's work on the configuration/storage section next") rather than letting the
agent pick its own order or attempt the whole thing in one pass. This keeps each unit of work
small enough to actually review, and keeps you in control of pacing the same way the document
checkpoints did in the previous phase.

**Verify real third-party API shapes instead of trusting training data.** Before writing code
against a library or framework, especially one that's changed across versions, have the agent
confirm the *actual* installed shape — method signatures, nullability, return types — rather
than relying on what a training set suggests. Reflection against the installed package (or
equivalent for your ecosystem), checking generated type stubs, or reading the library's real
source are all fair game. This catches real discrepancies — a namespace that doesn't actually
exist, a parameter that's required when documentation implies it's optional — before they ship
as bugs.

**Build and test after every phase, not just at the end.** Run your project's build (and test
suite, once one exists) after essentially every implementation step, with a standing rule of
zero new warnings before moving on. Problems caught immediately, while the context for fixing
them is still fresh, are far cheaper than the same problems discovered in a batch at the end.

**Do manual, end-to-end smoke tests of the actual running artifact**, not just automated tests.
Run the real command, hit the real endpoint, force the edge-case code paths deliberately (an
expired credential, a missing file, a stale cache) and confirm the whole vertical slice
actually behaves — not just that it compiles or that unit tests pass in isolation.

**Periodically ask "why," not just "what."** Every so often, directly question a decision that
already "works" — a design choice, a dependency pattern, an assumption baked in earlier — and
ask the agent to re-derive the reasoning from first principles rather than defend it
reflexively. This surfaces mistakes in the agent's own earlier reasoning that functional
testing alone won't catch; an explanation that sounded right when first given can turn out to
be subtly wrong once actually reasoned through carefully, and the fix belongs back in the
architecture document, not just in your head.

**Reconcile deviations back into the docs immediately, not "later."** When implementation
reveals a document was wrong or incomplete, this is the moment to apply the habit established
in [Phase 3](#phase-3-the-five-document-feature-lifecycle) — fix the code and the document in
the same breath, with a short note of what changed and why.

**Ask for commit messages at natural checkpoints, and commit them yourself.** A good checkpoint
is after a milestone's documents are finalized but before implementation begins, and again
after implementation and tests are done. Have the agent draft an accurate, well-structured
message describing everything that changed — but run `git commit` yourself. Keeping a human
hand on the actual commit is a deliberate control point, not a convenience the agent needs.

## Prompt Patterns for Future Firmware Work

Use these patterns to guide upcoming work in this repository. They are prospective examples,
not claims that firmware features, electrical checks, or validation have already happened. The
goal is to give Codex a bounded task, identify the evidence that governs it, and make the next
review point explicit.

| Discipline | Prompt that leaves too much implicit | More useful prompt for this repository |
| --- | --- | --- |
| **Begin with discussion, not files.** Establish behavior and milestone boundaries before creating specifications. | “Start the firmware.” | “Let’s discuss the first firmware milestone. Read `AGENTS.md`, `firmware/README.md`, `docs/status.md`, and the relevant hardware records. Ask focused questions about desired behavior and material unknowns. Keep this conversation-only: do not create files or write code.” |
| **Name the source of truth.** Point to maintained records instead of asking Codex to reconstruct decisions from conversation. | “Wire up the ADC the way we discussed.” | “For connectivity, use the W01 wiring source as authority. Read the ADC and RC-filter component READMEs and the relevant open questions. Summarize the documented interface and unresolved details before proposing firmware behavior; do not infer GPIO assignments.” |
| **Separate evidence from assumptions.** Say what is reported, measured, and still unverified; ask only for information that changes the design. | “The protoboard is built, so make it work.” | “The development protoboard has been reported as assembled. Treat that as reported assembly, not proof that every connection has passed continuity or power checks. Use published component information as the working basis where the project permits it; identify a check only if its result changes this milestone’s design or test.” |
| **Request one artifact and stop at the review gate.** Keep the five-document lifecycle genuinely reviewable. | “Write the spec, research, architecture, plan, and tasks for the gauge.” | “Create only the feature document for the approved milestone, using the repository template and current project records. Do not draft the research document or implementation yet. Stop after the feature document so it can be reviewed.” |
| **Bound implementation to an approved phase.** A broad implementation request can skip decisions and make results hard to evaluate. | “Implement the firmware and test everything.” | “Implement only Phase 1 in the approved `tasks.md`. Keep shared measurement logic independent of board-specific display code and follow the documented N16R8/XIAO distinction. Run the checks specified for this phase, report commands and results, distinguish build evidence from physical validation, update the affected documents, and stop.” |
| **Offer a concrete view when there is one.** A stated preference gives the review a useful starting point while leaving room to correct a real conflict. | “How should startup zeroing work?” | “I prefer not to zero automatically while the engine is running. Check this against the measurement design; identify any concrete conflict and propose a behavior that preserves that constraint.” |

The useful prompt is not necessarily longer. It makes the requested phase, authoritative files,
evidence status, constraints, and stopping point clear enough that Codex does not have to silently
choose them. Keep project-specific facts in their maintained documents; use the prompt to identify
which records govern the task and what work is authorized now.
## The Kickoff Prompt

Establishing this whole workflow with a fresh agent session doesn't have to mean re-explaining
every phase above from scratch. The block below is meant to be copied, filled in with your own
placeholders, and pasted as literally your first message to a new agent session. Its job is
narrow: get the agent committed to this discipline immediately, and prompt it to ask you
clarifying questions about anything you left vague — it isn't trying to teach the whole
philosophy inline; that's what the rest of this document is for, and you're welcome to share
this document alongside the prompt if you want the agent to have the full reasoning as context
too.

It also exists as its own file, `templates/kickoff-prompt.template.md`, if you'd rather copy it
from there than out of a rendered markdown block. That same `templates/` folder also holds a
lightweight skeleton for each of the five feature-lifecycle documents (`feature.template.md`,
`research.template.md`, `architecture.template.md`, `plan.template.md`, `tasks.template.md`) —
worth a look for your very first milestone, before you have a prior feature of your own to
point the agent at instead. They're starting points, not forms to fill in — expect your actual
documents to deviate from them, the same way ours did.

```text
I want to develop the firmware for this project using the spec-driven milestone workflow in
`firmware/docs/developing-with-codex/README.md`. Follow that process throughout firmware work.

Start with a conversation-only brainstorm. Read `AGENTS.md`, `README.md`, `docs/status.md`,
`firmware/README.md`, and relevant hardware sources. Ask focused questions about intended
behavior, constraints, physical assembly evidence and unresolved assumptions. Do not create or
modify files during the brainstorm.

After the brainstorm, stop and wait for explicit approval before writing the milestone list.
For each approved milestone, write exactly one document at a time in this order: `feature.md`,
`research.md`, `architecture.md`, `plan.md`, and `tasks.md`. Stop after each document for review
and explicit approval. Implement only after all five documents are approved and a specific task
phase is selected. Complete and verify only that phase, update the documents with discoveries,
report the result, and stop for direction.

Keep published claims, working assumptions, reported assembly, measured results and validated
behavior distinct. Ask before resolving a material design question by assumption.

For now, the goal is to discuss the firmware milestones. Do not start implementation.
```

## When This Is (and Isn't) Worth It

All of this pays off on the kind of work this document assumes: an existing, multi-feature
codebase where later work depends on decisions made earlier, and where a wrong assumption is
expensive to unwind. A one-off script, a throwaway experiment, or a change small enough to hold
entirely in your head doesn't need five documents and a phase-by-phase rollout — scale the
discipline down to match the stakes, not up out of habit.

## Further Reading

- [GitHub Spec Kit](https://github.com/github/spec-kit) — The open-source, spec-driven
  development toolkit (Specify → Plan → Tasks → Implement) cited in "Why This Works"; works
  for multiple coding agents.
- [Claude Code: Best Practices](https://code.claude.com/docs/en/best-practices) — Anthropic's
  published guidance on working with an agentic coding tool, including the context-window and
  instructions-file points cited in "Why This Works."
- [OpenAI: Iterating Development Workflows with Codex](https://developers.openai.com/cookbook/examples/codex/iterating-development-workflows-with-codex)
  — an official Codex workflow example. Its optional artifact structure is adapted rather than
  copied wholesale.

## Adaptation note

This guide adapts the five-document, milestone-based workflow used in a separate .NET project.
It retains the brainstorm, durable standards, ordered document reviews and implementation-phase
gates. Copilot-specific setup was replaced with Codex CLI and this repository's `AGENTS.md`;
examples and templates were rewritten for embedded firmware and the PCV gauge. The source
project's files and private paths are not part of this repository.
