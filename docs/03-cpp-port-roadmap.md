# 03 — C++ Port Roadmap

The plan to rebuild the C game (Doc 01) in C++, on SDL2 (Doc 02), one
**runnable slice** at a time.

## Two rules that make this work

1. **Every phase ends in something you can run.** No phase is "port module X."
   You always have a window you can open and a thing you can see or do. This is
   the opposite of a module-by-module port (which produces nothing playable
   until the very end).
2. **Every phase has a *dual* main goal** — a **Feature** milestone *and* a
   **C++ concept** milestone. The feature is the *what* (already fixed, from
   Doc 01); the C++ concept is the *why you're really here*. When the two
   conflict, the feature is the spec and the C++ concept is how you express it.

A third habit, not a rule: at each phase, hit the **reproduce-vs-fix** decisions
for any quirks that phase touches (Doc 01 §Catalogue). Default to *reproduce*
("emulation"); fix only on purpose, and write down which you chose.

---

## Phase map

| # | Feature milestone (the *what*) | C++ concept milestone (the *why*) |
|---|---|---|
| **0** | Window opens, clears, presents; clean exit | Build system (CMake), RAII wrappers for SDL init/window/renderer |
| **1** | Paddle + ball drawn; paddle moves with arrows | Classes & members; a `Renderer` wrapper; `enum class` |
| **2** | Ball flies and bounces off the 3 walls | Value types, references, a fixed-timestep loop |
| **3** | Brick grid; collisions destroy bricks; score counts | `std::array`/`std::vector`, range-for, a testable collision fn |
| **4** | Round ends (ball lost / all cleared) → game-over state | `enum class` state, ownership of game lifecycle |
| **5** | On-screen text + score HUD | RAII over `TTF_Font`/textures, `std::string`, a cache |
| **6** | Menus + screen state machine wired together | Polymorphism *or* `std::function`; killing the copy-paste |
| **7** | 3-slot save/load to disk | `std::fstream`, value-semantic structs, no more `strdup` |
| **8** | High-score table + name entry | `std::sort`, `std::vector<Entry>`, carry the unit tests over |
| **9** | The four music tracks | RAII over `Mix_Music`, an `AudioPlayer` class |
| **10** | Settings, auto-play, polish — *faithful build complete* | Final decomposition; retire the god-struct |
| **11+** | *(optional)* swap a backend / single-header libs | Prove your abstractions: rewrite wrappers, not the game |

Phases 0–10 reproduce the whole game. 11+ is the optional library track from
Doc 02.

---

## Phase 0 — Foundation: a window that opens and closes cleanly

**Feature:** run the program; an 864×558 window appears, fills with a colour,
and closes on ✕ or Esc without leaking or crashing.

**C++ concept — RAII, the cornerstone.** This is the most important lesson in
the whole project, so it's first. In C, `init_app`/`close_app` are ~120 lines of
manual `SDL_Init`/`SDL_CreateWindow`/…/`SDL_DestroyWindow`/`SDL_Quit`. In C++ you
write small wrapper classes whose **constructors acquire and destructors
release**:

```cpp
struct SDLContext {            // ctor: SDL_Init(...);  dtor: SDL_Quit();
    SDLContext();              // throws on failure
    ~SDLContext();
    SDLContext(const SDLContext&) = delete;   // non-copyable
};
// unique_ptr with custom deleters for the C handles:
using WindowPtr   = std::unique_ptr<SDL_Window,   decltype(&SDL_DestroyWindow)>;
using RendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
```

When `main` returns, everything tears down automatically, in the right order.
`close_app`'s entire teardown block disappears — *that* is the feeling to chase.

**Tasks:** set up CMake (`find_package(SDL2)`), `clang++ -std=c++20`; an
`Application` class owning `SDLContext`/window/renderer; a minimal loop
(poll events, clear, present, quit on Esc). The original's assets are already
in [`assets/`](../assets) (see Doc 05) — have CMake copy that folder next to the
binary (e.g. `file(COPY assets DESTINATION ...)`) so paths resolve from any cwd;
nothing is *loaded* until Phase 1, but wiring the path now saves churn later.

**Reproduce-vs-fix:** none yet.

**Done when:** `cmake --build build && ./build/breakout` opens and closes a blank
window cleanly (check with a leak sanitiser if you like).

---

## Phase 1 — Paddle & ball on screen, paddle moves

**Feature:** draw the paddle (bottom-center) and the ball (center) as filled
rects; Left/Right arrows slide the paddle, bounded by the side padding; Alt makes
it faster.

**C++ concept — your first real classes + a `Renderer` wrapper.** Introduce a
`Renderer` class wrapping `SDL_Renderer*` with `clear()`, `present()`,
`fill_rect(rect, color)`. Introduce a `Paddle` (and maybe `Ball`) class holding
an `SDL_Rect` and an `update(const Uint8* keys)` method. Use `enum class` if you
model anything stateful. This replaces "free functions taking `RenderContext*`"
with "methods on objects that own their data."

**Tasks:** `Renderer`, `Paddle`, `Ball`; read keyboard via
`SDL_GetKeyboardState`; render both rects each frame.

**Reproduce-vs-fix:** Quirk #2 (Alt = double speed). *Recommend reproduce* — it's
in the README as a feature. Implement it the same way (apply move twice) or
cleanly (a `speed * (alt?2:1)`); either is fine since behaviour matches.

**Done when:** you can slide the paddle wall-to-wall, faster with Alt.

---

## Phase 2 — Ball physics & wall bounces + the loop that makes it fair

**Feature:** the ball moves each frame and bounces off left/right (flip x) and
top (flip y) walls. Off the bottom it just leaves for now (Phase 4 handles
game-over).

**C++ concept — the fixed-timestep loop.** This is where you fix Doc 01 bug #9
(no fixed timestep → step-rate coupled to frame/CPU rate instead of wall-clock).
Build the canonical loop:
accumulate real elapsed time, step the simulation in fixed `dt` slices, render
the latest state. Use `SDL_GetTicks()` (simple) or `std::chrono::steady_clock`
(more STL). Now the ball moves the same speed on any machine.

```cpp
double accumulator = 0.0;
const double dt = 1.0 / 60.0;
while (running) {
    accumulator += frame_seconds();
    while (accumulator >= dt) { world.update(dt); accumulator -= dt; }
    world.render(renderer);
}
```

**Tasks:** a `World`/`Game` class owning ball+paddle+walls; `update(dt)` moves
and bounces the ball; wire in the fixed-timestep loop.

**Reproduce-vs-fix:** Quirk #3 (subtraction movement) — *keep* the resulting
direction (ball heads up-left), but you can implement it as normal addition with
negative initial velocity; what matters is it looks the same. Bug #9 — **fix**
(that's the whole point of this phase).

**Done when:** the ball bounces around the three walls forever at a steady,
machine-independent speed.

---

## Phase 3 — Bricks, collision, scoring

**Feature:** the 5×9 brick grid (top row health 5 … bottom row 1), drawn in
health-shaded grey; the ball damages a brick on contact (`health--`, bounce,
+1 score); the ball bounces off the paddle. Score shows in the title bar for now
(real HUD is Phase 5).

**C++ concept — STL containers + a unit-testable core.** Model the grid as
`std::array<std::array<Brick,9>,5>` (or `std::vector<Brick>`). Iterate with
range-for. Crucially, write collision as a **free function with no SDL
dependency** so you can unit-test it (mirroring how `scores.c` is the only
testable C module):

```cpp
bool intersects(const SDL_Rect& a, const SDL_Rect& b);   // testable, pure
```

**Tasks:** `Brick`; grid init (fresh layout, health `5 - row`); per-frame
brick/paddle collision with the anti-stick nudge; brick colour by health.

**Reproduce-vs-fix:** Quirk #4 (asymmetric anti-stick nudge), #5 (multi-brick
y-flips cancel) — *reproduce* for faithful feel; note them. The grey-shade
colour mapping — *reproduce*.

**Done when:** you can clear bricks, the score climbs, and the ball plays off the
paddle.

---

## Phase 4 — Round end & the game-over transition

**Feature:** dropping the ball past the bottom **or** clearing all bricks ends
the round and moves to a game-over state showing the final score (score in the
title bar / log for now — the real on-screen text HUD arrives in Phase 5).

**C++ concept — `enum class` state + lifecycle ownership.** Replace the
`status` magic numbers (−1/0/1) with `enum class GameStatus { Empty, Saved, Lost
};` (and decide whether to add `Won`). Model the round-over transition as a state
change the loop reacts to, not an in-place `SDL_Delay`.

**Reproduce-vs-fix — two notable decisions land here:**
- **Bug #8 (`SDL_Delay(2000)` freeze):** *recommend FIX.* A blocking 2-second
  freeze is bad and easy to replace with a non-blocking timed transition (record
  a timestamp, keep rendering, switch after 2s). Faithful *enough* (still a 2s
  pause) but the window stays alive.
- **Quirk #1 (win == lose):** *your call, and a fun one.* Faithful emulation says
  reproduce (a win is logged as a loss, no win screen). But this is the most
  natural place to *add* the missing win state if you want the game to feel
  "finished." Decide deliberately and write it down.

**Done when:** both end conditions land you on a game-over screen with the score.

---

## Phase 5 — Text rendering & the HUD

**Feature:** real on-screen text — the `[Score: N]` HUD during play, titles, and
menu labels — via the TTF font.

**C++ concept — RAII over font/textures + `std::string` + a cache.** A
`TextRenderer`/`Font` class owns the `TTF_Font*` (RAII). Text is `std::string`
throughout. Because the C re-renders every string into a fresh texture every
frame, add a simple **string→texture cache** (`std::unordered_map<std::string,
Texture>`); this is a clean, motivated use of an STL map and teaches you why
caching matters.

**Reproduce-vs-fix:** quirk #18 (`exit(1)` in the font helper) — **fix** (throw
or return an error; don't kill the app from a draw helper). Word-wrap (Doc 01
§4.3) — reproduce with `std::string`/`std::istringstream` instead of `sscanf`.

**Done when:** the score HUD and any title text render correctly.

---

## Phase 6 — Menus & the screen state machine

**Feature:** main menu, and the navigation skeleton between screens (menu ⇄ game
⇄ quit menu). Buttons you can click.

**C++ concept — kill the copy-paste; behaviour over labels.** The C has ~7
near-identical screen loops dispatching by `strcmp` on button labels. Replace
with **one** reusable abstraction. Two idiomatic options — pick one to *learn*
that technique:
- **`std::function` callbacks:** `struct Button { SDL_Rect rect; std::string
  text; std::function<void()> on_click; };` in a `std::vector<Button>`; a `Menu`
  runs the loop and calls the clicked button's lambda. (Closer to the existing
  shape; less ceremony.)
- **Polymorphism:** an abstract `Screen` base with `virtual ScreenState run()`;
  each screen a subclass. (More OO; teaches virtual dispatch + ownership via
  `std::unique_ptr<Screen>`.)

Either way: the click action is carried by the button/screen, **not recovered
from its display text**, and the slot index is stored *as data*, not parsed out
of `"Slot %d"`.

**Tasks:** `Button` value type + `Menu`/`Screen`; main menu; the
`handle_screen_state` equivalent (keep the iterative, return-to-top shape from
Doc 01 — it's the fix for the old recursion).

**Reproduce-vs-fix:** quirk #12 (`SCREEN_KEYBOARD_QUICK` dead enum) — just don't
port it. Settings-toggle-by-relabeling (Doc 01 §4.8) — *fix* (hold a bool, redraw
from it) rather than reproduce the relabel trick.

**Done when:** you can navigate menu→game→quit→back with clickable buttons and
zero duplicated loop code.

---

## Phase 7 — Save / load (3 slots)

**Feature:** the load-game and save-game screens; three slots persisted to
`savefile.txt`; fresh/continue/load decision logic; "reset save file."

**C++ concept — value semantics retire `strdup`.** Once `GameState::name` is a
`std::string`, copying a slot (`saved_games[i] = active_game`) is a correct deep
copy — Doc 01 bug #10 (shared `char*` aliasing) **can't happen**. This is the
single most convincing "C++ memory model > manual C" moment in the project. File
I/O via `std::ifstream`/`ofstream`. Put the fresh/continue/load decision tree on
a `SaveManager` and **unit-test it** (it's pure logic).

**Reproduce-vs-fix:** the fragile space-separated single-line format (Doc 01
§Data formats) — *recommend* a tiny robustness fix (one field per line, or guard
names) since `std::string` names could contain spaces; or go further with JSON
(Doc 02) as a serialization lesson. Bug #11 (names-with-spaces) is auto-handled
if you change the format. `temp_collision` (#14) — drop it.

**Done when:** you can save a game to a slot, quit, relaunch, and load it back.

---

## Phase 8 — High scores & name entry

**Feature:** the on-screen keyboard to enter a name, the top-3 high-score table,
DJB2 hash verification, and recording a score on game-over-with-a-slot.

**C++ concept — `std::sort` + carry the tests over.** `Entry` becomes a
value-type struct (`std::string name, hash; int score; std::time_t ts;`); the
deep-copy-into-temp-array-of-4 dance in `update_high_scores` becomes
`std::vector<Entry>` + `std::sort(…, [](auto&a,auto&b){return a.score>b.score;})`
+ `resize(3)`. The 12 existing Unity tests map almost 1:1 onto **Catch2/doctest**
— porting them is a quick win and proves the logic survived.

**Reproduce-vs-fix:** keep DJB2 + hash verification (#... active hash check) for
faithfulness. Name entry: *consider fixing* to SDL's `SDL_TEXTINPUT` event path
(Doc 01 §4.7) instead of the scancode-scan + 200ms-delay hack — cleaner and
correct, but a behaviour change, so decide deliberately. Quirk #6 (default
uppercase) — reproduce or fix per taste. Dead funcs (#13) — don't port.

**Done when:** finishing a saved game prompts for a name and the score lands in a
correctly-sorted top-3 that persists.

---

## Phase 9 — Audio

**Feature:** lobby music on menus, normal in-game track, intense track after
score > 20, the "ba-dum-tss" on game over.

**C++ concept — RAII over `Mix_Music` + a small state machine in a class.** An
`AudioPlayer` owns the four tracks (`unique_ptr<Mix_Music,…>`), with
`play(Track)`, `pause()`, `resume()`. The tri-state `is_playing` int (0/1/2) and
`MusicTrack` become `enum class`. The branchy `play_music` logic (Doc 01 §4.11)
becomes one tidy method.

**Reproduce-vs-fix:** the one-way intense-music latch and the inconsistent
fade-vs-instant choice — reproduce for faithfulness (or make fade a per-track
property, which is both cleaner *and* behaviour-compatible).

**Done when:** music switches correctly across menus, gameplay, the intensity
threshold, and game over.

---

## Phase 10 — Settings, auto-play, and final decomposition

**Feature:** the settings screen, the persisted `auto_play` flag, and auto-play
(attract) mode driving the paddle. With this, **the faithful emulation is
complete.**

**C++ concept — retire the god-struct.** By now you have `Renderer`,
`AudioPlayer`, `TextRenderer`, `World`, `SaveManager`, `HighScoreTable`,
`Menu`/`Screen`. The final step is making sure each *owns* its data with a narrow
interface, so the old `AppContext` (passed everywhere) is gone — replaced by an
`Application`/`Game` that composes the systems. `Settings` becomes `struct
Settings { bool auto_play; }` via a typed config helper.

**Reproduce-vs-fix:** Bug #7 (dead auto-play clamp → paddle leaves screen) —
*recommend FIX* (actually clamp), since leaving the screen looks broken; but
faithful emulation could keep it. Decide and note it.

**Done when:** the full game plays start-to-finish identically to the C original
(modulo the fixes you deliberately chose), and no single god-struct remains.

---

## Phase 11+ — Optional: backend / library migration

Only after 0–10 are done and faithful. Because SDL lives behind your `Renderer`
/ `AudioPlayer` / `TextRenderer` classes, swapping backends means rewriting
*those classes*, not the game. Good isolated exercises, in rough order of effort:
- Swap single-header libs first: `SDL2_image` → **`stb_image`**, `SDL2_mixer` →
  **`miniaudio`** (Doc 02). Each touches one class.
- Then, if you want, a whole-stack move: **SDL3** (smallest delta; brings float
  rects), **SFML** (most C++-native), or **raylib** (least boilerplate).
- If your wrappers were well-designed, the game logic (Phases 2–4, 7, 8)
  shouldn't change at all. That's the test of a good abstraction — and the payoff
  for hiding SDL in Phase 0.

---

## Suggested cadence

- Phases **0–4** are the spine — do them in order; each is small and you get a
  *playable* ball-and-bricks by Phase 3. Don't skip ahead to menus.
- Phases **5–6** make it feel like a real app.
- Phases **7–10** add the surrounding features; their order is flexible (save,
  scores, audio, settings are fairly independent) — but doing **7 before 8** is
  natural (scores piggyback on the save/name machinery).
- After **each** phase: commit, run it, and jot the reproduce-vs-fix decisions
  you made. That log becomes your "what I changed and why" record — and great
  evidence of what you learned.
