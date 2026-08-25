# Breakout — C++ Rewrite

>> Following Text is written by an LLM after conversing a few times back and forth , Take all first person language with a pinch of salt

A from-scratch C++ rewrite of [`../Breakout_C`](https://github.com/Collaboration95/Breakout_C.git), a Breakout-style
arcade game originally written in **C + SDL2** as a university project
(50.051 Programming Language Concepts, 2024).

The goal of this project is **learning modern C++** by faithfully re-emulating a
game I already understand deeply. Because I know exactly how the original
behaves, every phase becomes a clean lesson: the *what* (the feature) is fixed,
so all the new thinking goes into the *how* (idiomatic C++).

> **Status:** Planning / documentation. No C++ code exists yet — but this is now
> its own git repo holding the analysis, the roadmap, **and the original game's
> assets** (`assets/`: font, textures, music — copied byte-for-byte), so the
> rewrite can start with the exact look and sound already in place. The large
> music WAVs are tracked with **Git LFS** (see `docs/05`) to keep history light.

## The documents

Read them in order. They go from "understand what exists" → "decide the tools"
→ "build it back, one runnable slice at a time."

| Doc | What it is | When to read |
|-----|-----------|--------------|
| [`docs/01-c-game-reference.md`](docs/01-c-game-reference.md) | **Complete reference** for the existing C game: architecture, every subsystem, every data format, and a full catalogue of quirks/bugs/dead-code. Each subsystem ends with a **C → C++ mapping**. | First. This is the spec we are re-implementing. |
| [`docs/02-library-alternatives.md`](docs/02-library-alternatives.md) | The **library landscape**: SDL2 vs SDL3 vs SFML vs raylib, plus per-subsystem swaps (images, audio, fonts, the score hash). With a recommendation. | Second, before locking in tools. |
| [`docs/03-cpp-port-roadmap.md`](docs/03-cpp-port-roadmap.md) | The **phased plan**. Each phase is a runnable slice with a dual goal: a *feature* milestone **and** a *C++ concept* milestone. | Third, then keep open while building. |
| [`docs/04-constants-and-layout.md`](docs/04-constants-and-layout.md) | The **"never open the source" numeric reference**: every constant, exact wall/brick/paddle geometry (pre-computed), colours, fonts, and per-screen button positions. | Lookup while building any phase. |
| [`docs/05-assets-and-resources.md`](docs/05-assets-and-resources.md) | **Asset manifest + visual reference**: which files the game uses vs. ships-but-ignores, sizes/roles, screenshots of the screens, and the carry-over status. The used assets are **already copied into [`assets/`](assets)**. | When wiring up asset paths (Phase 0/1). |

> Docs 01–03 = understand + plan. Docs 04–05 = the exact numbers and assets, so
> you never have to reopen `Breakout_C/` to rebuild it faithfully.

## The original, in one breath

Paddle at the bottom, a ball that bounces off three walls and a 5×9 grid of
bricks. Each brick has 1–5 health (shown as shades of grey); clear them all or
miss the ball and the game ends. Wrapped around the core loop: a main menu,
3-slot save/load, a top-3 high-score table with name entry, a settings screen
with an auto-play (demo) mode, and four music tracks that swap by game state.

~3,300 lines of C across 11 well-separated modules, all state threaded through a
single `AppContext` struct.

## Guiding principle: emulate first, modernise in *how*, not *what*

"Final emulation" means the C++ version should **behave like the original** —
same feel, same screens, same quirks. The original's bugs and oddities (dead
auto-play clamp, double-speed Alt key, win == lose, blocking 2-second freeze on
game-over, …) are catalogued in Doc 01. For each, the roadmap calls out a
**reproduce-vs-fix decision** so the choice is deliberate, never accidental.

The C++ improvements are in *how the code is written*: RAII over manual
`free()`, `std::string` over `char*`, `std::vector`/`std::array` over raw
arrays, `enum class` over magic numbers, classes with clear ownership over the
`AppContext` god-struct.
