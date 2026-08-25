# Current Progress / Resume Note

This is the durable handoff note for returning after a gap. Keep it short and
update it at the end of a coding session. It is intentionally separate from the
issue briefs, which should remain stable.

## Current snapshot (BRK-001 baseline — 2026-08-25)

- **Remote repository:** [Collaboration95/breakout-cpp](https://github.com/Collaboration95/breakout-cpp).
- **GitHub Project:** [Breakout C++ Rewrite](https://github.com/users/Collaboration95/projects/6).
- **Proposed sprint:** S0 — Foundation.
- **Proposed next issue:** [BRK-001](https://github.com/Collaboration95/breakout-cpp/issues/10) — Establish a truthful build and run baseline.
- **Planning status:** awaiting audit; no issue is Ready yet.

- **Repository state:** SDL tutorial scaffold (`src/main.cpp:8-9` — 640×480 `SDL_Window` + `SDL_Surface` via `SDL2_image` PNG `src/main.cpp:11`) — *not* the documented 864×558 target.
- **Documented target:** 864×558 per `docs/04-constants-and-layout.md:18` and `docs/03-cpp-port-roadmap.md:48` (from BRK-002 onward).
- **SDL deps currently used:** `SDL2` (2.32.70) + `SDL2_image` (2.8.12) — `CMakeLists.txt:36-37` / `src/main.cpp:2-3`, required via `find_package(REQUIRED)` so missing deps fail loudly.
- **Canonical build:** `cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build` from repo root — CMake is source of truth; `make` is convenience wrapper only (`decision-ledger.md:11`).
- **Toolchain:** Auto-discovered `AppleClang 21.0.0` (`/usr/bin/c++`); C++20 (`CMakeLists.txt:5`); no required Homebrew path.
- **Launch assumptions:** `./build/breakout` **from repo root only** — assets resolved as `assets/background-panel.png` relative to CWD (`src/main.cpp:11`). Launching from `build/` or another CWD fails (`No such file`). No `file(COPY assets ...)` yet (deferred to BRK-013). Window is 640×480 scaffold; draws after event loop with 30s `sleepForXSeconds` (`src/main.cpp:114-125`) — known scaffold limitation, fixed in BRK-002.
- **Phase status:** Scaffold only — BRK-002 (continuous 864×558 loop) and BRK-003 (RAII ownership) not started. This note distinguishes scaffold from first gameplay slice per `BRK-001.md:57`.
- **Last known-good build:** 2026-08-25 / `cdc8525` / AppleClang 21.0.0 / SDL2 2.32.70 + SDL2_image 2.8.12 / `build/breakout` (65K) — verified `cmake -B build && cmake --build build` succeeds from clean `build/`.

## Session handoff template

```text
Date:
Issue:
What changed:
Acceptance criteria checked:
Verification commands/manual checks:
Decision recorded:
Known problem or blocker:
Next concrete action:
Last known-good commit:
```

## Rules for this note

- The next action must be small enough to begin without reconstructing context
  from memory.
- If the working tree is not in a known state, say so explicitly.
- If a design choice affects behavior, file compatibility, or the learning goal,
  copy it into `decision-ledger.md` rather than leaving it only here.
- This note is not a substitute for issue acceptance evidence.
