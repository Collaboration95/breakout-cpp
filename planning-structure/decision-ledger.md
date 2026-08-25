# Decision Ledger

These are deliberate choices for the human owner. The backlog should not hide a
new product or learning decision inside an implementation issue. Record the
choice, date, rationale, and affected issue keys before moving dependent work to
`Ready`.

| Decision | Current status | Affected issues | Question to answer |
|---|---|---|---|
| C++ language level | Decided 2026-08-25 | BRK-001 onward | C++20 — matches Doc02/03 roadmap |
| Canonical build entry point | Decided 2026-08-25 | BRK-001 | CMake is canonical; Makefile delegates to `cmake --build build` |
| Toolchain portability | Decided 2026-08-25 | BRK-001 | Discover compiler normally (`CXX ?= c++`); no required `/opt/homebrew/bin/g++-16` |
| Fidelity profile | Open | BRK-007 onward | Which player-visible behaviors must match, and which technical defects may be fixed? |
| Win behavior | Open | BRK-012 | Preserve the original win-as-loss path, or introduce a distinct victory outcome? |
| Round-end timing | Open | BRK-012 | Preserve the two-second pause's duration while keeping the window responsive, or reproduce the freeze exactly? |
| Physics representation | Open | BRK-007 onward | Keep integer SDL geometry for the model, or use a separate numeric representation for simulation? |
| Collision edge cases | Open | BRK-009, BRK-011 | Preserve asymmetric anti-stick and multi-brick bounce quirks, or define cleaner collision behavior? |
| Asset root resolution | Open | BRK-013 | Should assets resolve from the working directory, executable location, or an explicitly configured project root? |
| Save format | Open | BRK-020 | Preserve the original space-separated format for compatibility, or use a more robust format for names and validation? |
| Name input behavior | Open | BRK-021 | Reproduce uppercase/default and delay behavior, or use SDL text input semantics? |
| UI behavior model | Open | BRK-016, BRK-017 | Use action identifiers, callbacks, or another approach while keeping display text non-authoritative? |
| Testing approach | Open | BRK-011, BRK-020, BRK-023 | Start with a small standard-library test executable, adopt a framework, or retain focused manual checks initially? |
| Audio failure policy | Open | BRK-026 | Is missing audio fatal, warned-and-disabled, or handled per resource? |

## Default policy while decisions are open

Do not make a decision merely to unblock a ticket if the choice changes player
behavior, file compatibility, or the learning objective. Mark the issue
`Blocked`, write the question here, and escalate. Small implementation choices
that do not affect those boundaries remain the learner's decision and do not
need pre-approval.

