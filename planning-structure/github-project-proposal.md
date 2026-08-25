# Proposed GitHub Project

## Project identity

**Name:** Breakout C++ Rewrite

**Purpose:** Provide a small, durable execution system for rebuilding the known
Breakout game in C++ with SDL2 while learning ownership, value semantics,
timing, input, rendering, persistence, and testing. The board should answer two
questions immediately: “What sprint am I in?” and “What is the smallest issue I
can work on next?”

**Repository:** `Breakout_cpp`

**Working constraint:** approximately two hours per week. The project favors
small, independently verifiable issues over large phase-level tickets.

**Remote status:** applied to
[Project #6](https://github.com/users/Collaboration95/projects/6) and the
`Collaboration95/breakout-cpp` repository. The local packet remains the planning
source for future reconciliation.

## Product scope

The initial release is a recognizable, playable C++/SDL2 rewrite of the C game:
window lifecycle, paddle and ball, brick physics, score, game-over flow, text,
menus, save/load slots, high scores, music, settings, and auto-play. It should
preserve the original's visible geometry, assets, controls, and intended screen
flow unless a deliberate decision records a change.

The project does not require a line-by-line translation, an immediate final
architecture, or an early switch to SDL3, SFML, raylib, JSON, or alternate audio
libraries. Those are possible post-release exercises.

## Proposed fields

| Field | Values | Use |
|---|---|---|
| Status | Backlog, Ready, In progress, Review, Blocked, Done | Current work state |
| Iteration | S0 Foundation through S9 Completion | Sprint/timebox |
| Priority | P0 Critical, P1 Important, P2 Later | Ordering inside a sprint |
| Size | S, M, L | Expected focused-session size; L requires audit |
| Risk | Low, Medium, High | Review intensity and escalation |
| Area | Tooling, Runtime, Rendering, Input, Simulation, UI, Persistence, Scores, Audio, QA | Filtering |
| Milestone | Foundation, Playable Core, Game Shell, Persistence, Faithful Release | Release-level grouping |

Use `Blocked` for a real dependency or external/tooling problem, not for a
large issue that should have been split.

## Proposed views

### Roadmap

Group by Milestone and sort by Iteration, Priority, then Size. This shows the
large outcome sequence without hiding the atomic issues.

The Project also retains an `All work` table view as the unfiltered default.

### Current iteration

Filter to the active Iteration. Group by Status and sort by dependency order.
This is the main weekly coding view.

### Ready queue

Filter `Status = Ready`, hide completed work, and sort by Priority then
dependency order. Only issues with all prerequisites satisfied should appear.

### Review queue

Filter `Status = Review`. Include the issue's verification evidence in the pull
request or local handoff before moving it to Done.

### Blocked

Filter `Status = Blocked`, showing the blocking issue or decision in the first
visible text field. A blocked issue should never be the only record of an
unresolved design choice; the decision belongs in the ledger too.

## Proposed labels

Use a small, stable label set rather than reproducing every concept as a label:

- `type:epic`, `type:spike`, `type:implementation`, `type:verification`, `type:docs`
- `area:tooling`, `area:runtime`, `area:rendering`, `area:input`,
  `area:simulation`, `area:ui`, `area:persistence`, `area:scores`,
  `area:audio`, `area:qa`
- `priority:p0`, `priority:p1`, `priority:p2`
- `risk:low`, `risk:medium`, `risk:high`
- `status:blocked` only if the Project status field is not sufficient for the
  chosen GitHub workflow

## Proposed issue conventions

Every issue has a stable key such as `BRK-007` and a stable text key:

`breakout-cpp:issue=brk-007`

Titles should describe the outcome in imperative language. Issue bodies should
retain the following sections: objective, value, scope, requirements, acceptance
criteria, verification, dependencies, risks/decisions, and stop conditions.

Parent epics are outcome containers. They are not substitutes for child issue
acceptance criteria and should not be worked on directly.

## Proposed milestones

| Milestone | Included iterations | Exit meaning |
|---|---|---|
| Foundation | S0 | The program builds, runs, renders continuously, exits cleanly, and owns SDL resources safely. |
| Playable Core | S1–S3 | A player can move the paddle, play a deterministic brick round, score, and reach game-over. |
| Game Shell | S4–S5 | The core has the original visual language, text, buttons, and screen navigation. |
| Persistence | S6–S7 | Save slots and high scores survive relaunch with validated data. |
| Faithful Release | S8–S9 | Audio, settings, auto-play, fidelity decisions, and regression evidence are complete. |

## Readiness and completion

An issue may move to `Ready` only when its dependencies are Done, its decisions
are resolved, and its verification path is credible. An issue may move to `Done`
only when its observable acceptance criteria are met, the relevant manual or
automated checks are recorded, and no unrelated cleanup has been smuggled into
the change.
