# Sprint Plan

Each sprint is a related group of issues, not a promise that every listed issue
will finish in one week. The intended iteration is two weeks, with about four
hours available. Plan for two completed issues; treat any additional issue as
stretch work.

## S0 — Foundation

**Outcome:** a truthful, repeatable SDL application baseline.

**Issues:** BRK-001, BRK-002, BRK-003

**Exit gate:** a clean documented build launches an 864×558 application that
renders continuously, exits through close/Esc, and has explicit ownership and
failure cleanup for the SDL resources it uses.

## S1 — First visible objects

**Outcome:** the player can see and control the first game objects.

**Issues:** BRK-004, BRK-005, BRK-006

**Exit gate:** the renderer can draw the basic geometry; the paddle obeys the
documented controls and boundaries; the ball is represented at its documented
starting position.

## S2 — Deterministic physics

**Outcome:** movement and basic collision are independent of machine spin rate.

**Issues:** BRK-007, BRK-008, BRK-009

**Exit gate:** the ball moves using a stable simulation cadence, bounces from
the three intended walls, and interacts with the paddle without becoming
permanently stuck.

## S3 — Bricks and round lifecycle

**Outcome:** the core is a playable Breakout round.

**Issues:** BRK-010, BRK-011, BRK-012

**Exit gate:** the documented 5×9 brick field appears with health-dependent
shading, collision changes health and score, and either end condition reaches a
non-blocking game-over state.

## S4 — Presentation

**Outcome:** the playable core looks and communicates like the original.

**Issues:** BRK-013, BRK-014, BRK-015

**Exit gate:** the carried-over background and font are loaded through a clear
asset boundary, text renders safely, and the player can see instructions, score,
titles, and game-over information.

## S5 — Screen and interaction shell

**Outcome:** the game has a maintainable menu/state flow.

**Issues:** BRK-016, BRK-017, BRK-018

**Exit gate:** main menu, quick game, game, quit, back, and game-over routes work
without using display text as the source of behavior or recursive screen calls.

## S6 — Save slots and names

**Outcome:** game state can be named and persisted.

**Issues:** BRK-019, BRK-020, BRK-021, BRK-022

**Exit gate:** empty, active, lost, saved, and malformed slot states are
handled deliberately; a user can name, save, quit, relaunch, and load a slot.

## S7 — High scores

**Outcome:** scores are sorted, validated, persisted, and displayed.

**Issues:** BRK-023, BRK-024, BRK-025

**Exit gate:** saved-game completion can produce a validated top-three entry and
the table survives relaunch without importing incompatible legacy hashes.

## S8 — Audio and settings

**Outcome:** the surrounding game behavior is complete.

**Issues:** BRK-026, BRK-027, BRK-028, BRK-029

**Exit gate:** menu, normal game, intense game, and game-over audio transitions
work; settings persist; auto-play behaves intentionally within the field.

## S9 — Verification and handoff

**Outcome:** the rewrite has evidence and an explicit fidelity story.

**Issues:** BRK-030, BRK-031, BRK-032

**Exit gate:** reproduce-vs-fix decisions are recorded, resource/error paths are
reviewed, and the end-to-end manual regression path is repeatable.

## Critical path

`BRK-001 → BRK-002 → BRK-003 → BRK-004 → BRK-005/006 → BRK-007 → BRK-008 →
BRK-009 → BRK-010 → BRK-011 → BRK-012 → BRK-015 → BRK-017 → BRK-018`

Persistence, scores, audio, and settings branch from that spine only after the
core shell is stable. Avoid starting an optional library migration before S9.
