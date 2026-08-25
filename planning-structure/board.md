# Local Board Snapshot

This file is the local equivalent of opening GitHub Projects and asking “what
do I work on now?” It is intentionally provisional until the audit passes.

## Current position

**Current sprint:** S0 — Foundation

**GitHub Project:** [Breakout C++ Rewrite](https://github.com/users/Collaboration95/projects/6)

**Current issue:** [BRK-001 / Issue #10](https://github.com/Collaboration95/breakout-cpp/issues/10) — Establish a truthful build and run baseline

**Board status:** Proposed; do not treat any issue as Ready until the audit is
complete.

**Why this is first:** the repository currently contains an SDL tutorial
scaffold and uncommitted changes. The remaining backlog assumes a stable,
repeatable build/run contract and an honest starting point.

## Issue board

| Key | Sprint | Area | Size | Dependencies | Proposed status |
|---|---|---|---|---|---|
| BRK-001 | S0 | Tooling | S | — | Proposed |
| BRK-002 | S0 | Runtime | S | BRK-001 | Backlog |
| BRK-003 | S0 | Runtime | M | BRK-002 | Backlog |
| BRK-004 | S1 | Rendering | S | BRK-003 | Backlog |
| BRK-005 | S1 | Input | S | BRK-004 | Backlog |
| BRK-006 | S1 | Simulation | S | BRK-004 | Backlog |
| BRK-007 | S2 | Simulation | M | BRK-005, BRK-006 | Backlog |
| BRK-008 | S2 | Simulation | S | BRK-007 | Backlog |
| BRK-009 | S2 | Simulation | S | BRK-008 | Backlog |
| BRK-010 | S3 | Simulation | M | BRK-009 | Backlog |
| BRK-011 | S3 | Simulation | M | BRK-010 | Backlog |
| BRK-012 | S3 | Runtime | M | BRK-011 | Backlog |
| BRK-013 | S4 | Rendering | S | BRK-003 | Backlog |
| BRK-014 | S4 | Rendering | M | BRK-013 | Backlog |
| BRK-015 | S4 | UI | S | BRK-012, BRK-014 | Backlog |
| BRK-016 | S5 | UI | S | BRK-014 | Backlog |
| BRK-017 | S5 | UI | M | BRK-016 | Backlog |
| BRK-018 | S5 | UI | M | BRK-017, BRK-015 | Backlog |
| BRK-019 | S6 | Persistence | M | BRK-012 | Backlog |
| BRK-020 | S6 | Persistence | M | BRK-019 | Backlog |
| BRK-021 | S6 | Input | M | BRK-014, BRK-017 | Backlog |
| BRK-022 | S6 | Persistence | M | BRK-020, BRK-021, BRK-018 | Backlog |
| BRK-023 | S7 | Scores | M | BRK-019 | Backlog |
| BRK-024 | S7 | Scores | M | BRK-023 | Backlog |
| BRK-025 | S7 | UI | M | BRK-022, BRK-024, BRK-018 | Backlog |
| BRK-026 | S8 | Audio | M | BRK-003, BRK-013 | Backlog |
| BRK-027 | S8 | Audio | M | BRK-026, BRK-018 | Backlog |
| BRK-028 | S8 | Runtime | M | BRK-022, BRK-018 | Backlog |
| BRK-029 | S8 | Simulation | S | BRK-028, BRK-012 | Backlog |
| BRK-030 | S9 | QA | S | BRK-012, BRK-015, BRK-018, BRK-025, BRK-029 | Backlog |
| BRK-031 | S9 | QA | M | BRK-003, BRK-020, BRK-026 | Backlog |
| BRK-032 | S9 | QA | M | BRK-030, BRK-031 | Backlog |

## Weekly operating rule

Only one issue should be `In progress`. A second issue may be `Ready`, but it
should not be started until the first issue reaches a stopping point. If an
issue takes more than two focused sessions, stop and ask whether it should be
split rather than quietly expanding its scope.
