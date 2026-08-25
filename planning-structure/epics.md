# Proposed Parent Issues / Epics

These are parent-issue proposals for the eventual Project. They organize the
atomic child issues; they are not direct coding tasks.

## EPIC-01 — Establish a dependable C++/SDL foundation

**Objective:** Turn the current tutorial scaffold into a truthful, repeatable
starting point for the rewrite.

**Child issues:** BRK-001, BRK-002, BRK-003

**Complete when:** the documented build command works, the application renders
continuously at the intended window size, close/Esc exits cleanly, and SDL
resources have clear ownership and failure cleanup.

## EPIC-02 — Build the first visible game objects

**Objective:** Establish a small rendering and input boundary that can draw and
control the paddle and ball without committing to the final architecture.

**Child issues:** BRK-004, BRK-005, BRK-006

**Complete when:** the paddle responds to the original controls within its
boundaries and the ball is represented and drawn at the documented starting
position.

## EPIC-03 — Make the core simulation deterministic and playable

**Objective:** Reconstruct movement, collision, bricks, scoring, and round end
behavior as an independently understandable game core.

**Child issues:** BRK-007, BRK-008, BRK-009, BRK-010, BRK-011, BRK-012

**Complete when:** a player can play a round with stable timing, wall/paddle/
brick interactions, score changes, and a visible game-over transition.

## EPIC-04 — Restore the visual language and feedback

**Objective:** Replace debug-only feedback with the carried-over background,
font, text, HUD, and core screen presentation.

**Child issues:** BRK-013, BRK-014, BRK-015

**Complete when:** the core game and game-over state communicate their status on
screen using the documented assets, dimensions, and text hierarchy.

## EPIC-05 — Add a maintainable screen and interaction shell

**Objective:** Recreate the menu flow while keeping interaction behavior
separate from display labels and keeping the state machine iterative.

**Child issues:** BRK-016, BRK-017, BRK-018

**Complete when:** main menu, quick game, game, quit, back, and game-over routes
work through one coherent screen model.

## EPIC-06 — Persist game state without manual-memory hazards

**Objective:** Add value-semantic save slots, validated persistence, name entry,
and save/load flows.

**Child issues:** BRK-019, BRK-020, BRK-021, BRK-022

**Complete when:** all three slots can be created, saved, loaded, and recovered
from malformed or missing data without aliasing or silent corruption.

## EPIC-07 — Preserve and display high scores

**Objective:** Rebuild the pure score table, integrity check, persistence, and
game-over/high-score interaction.

**Child issues:** BRK-023, BRK-024, BRK-025

**Complete when:** eligible saved-game scores enter a validated sorted top-three
table and survive relaunch.

## EPIC-08 — Complete audio and settings behavior

**Objective:** Add owned audio resources, track transitions, persisted settings,
and auto-play.

**Child issues:** BRK-026, BRK-027, BRK-028, BRK-029

**Complete when:** menu/game/intense/game-over audio and the settings-driven
auto-play behavior work without taking ownership away from the application.

## EPIC-09 — Verify the rewrite and record intentional differences

**Objective:** Establish evidence that the rewrite is coherent, leak-safe, and
deliberately faithful where it matters.

**Child issues:** BRK-030, BRK-031, BRK-032

**Complete when:** the fidelity ledger is closed or consciously deferred, error
paths and ownership have been reviewed, and the full manual regression path is
recorded.
