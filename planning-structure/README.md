# Breakout C++ — Local Project Planning Structure

This folder is the planning source for the GitHub Project, its iterations, parent
epics, and atomic issue briefs. It was initially created as a local-only
proposal; that proposal has now been applied to
[Collaboration95/breakout-cpp](https://github.com/Collaboration95/breakout-cpp)
and [Project #6](https://github.com/users/Collaboration95/projects/6).

The project is a from-scratch C++/SDL2 rewrite of `Breakout_C`, primarily as a
personal C++ and SDL2 learning exercise. The owner writes the implementation
code. These documents define objectives and observable requirements; they do
not prescribe classes, method names, file layouts, or one “correct” design.

## How to use this folder

1. Open [`board.md`](board.md) to find the proposed current sprint and next
   issue.
2. Read that issue and its dependencies. Read only the relevant section of the
   source/reference docs linked from the issue.
3. Before coding, resolve any decision marked as a prerequisite in
   [`decision-ledger.md`](decision-ledger.md).
4. Work on one issue at a time. The issue's acceptance criteria are the stopping
   point for the session.
5. At the end of the session, record the result, verification, decision, and
   next resume point in the eventual project progress note.

The documents are ordered by purpose:

- [`github-project-proposal.md`](github-project-proposal.md) — proposed Project
  fields, statuses, views, labels, and working rules.
- [`epics.md`](epics.md) — proposed parent issues and outcome boundaries.
- [`sprint-plan.md`](sprint-plan.md) — dependency-aware iteration plan.
- [`board.md`](board.md) — the practical “what do I work on now?” snapshot.
- [`progress.md`](progress.md) — the short durable handoff note for returning
  after a gap.
- [`decision-ledger.md`](decision-ledger.md) — choices owned by the human
  learner rather than silently made by the backlog.
- [`audit-brief.md`](audit-brief.md) — checklist for the auditing agent.
- [`issues/`](issues/) — one brief per atomic implementation or verification
  issue.

## Source-of-truth boundaries

The existing project docs remain the behavioral reference:

- `docs/01-c-game-reference.md` describes the original game's behavior and
  quirks.
- `docs/02-library-alternatives.md` records the library strategy.
- `docs/03-cpp-port-roadmap.md` gives the broad learning direction.
- `docs/04-constants-and-layout.md` gives exact geometry and tunables.
- `docs/05-assets-and-resources.md` describes the carried-over assets.

This folder turns that broad plan into a resumable execution system. If an issue
conflicts with the reference docs, do not silently code around the conflict:
record the discrepancy in the decision ledger or send it to audit.

## Planning rules

- A sprint is a two-week iteration with roughly four hours of planned capacity.
- Plan for at most two completed issues per sprint. Any third issue is stretch
  work and may roll forward without being considered a failure.
- An issue should fit one focused coding session and one reviewable change.
- Issues describe outcomes and constraints, not implementation recipes.
- No issue is Ready while it has an unresolved product decision, missing
  dependency, ambiguous acceptance check, or no credible verification path.
- Optional backend/library migration is outside the initial project.
- Keep the remote Project and local packet aligned; do not make further broad
  remote changes without an audit or an explicit owner decision.
