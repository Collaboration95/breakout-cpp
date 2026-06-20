# 01 — The C Game: Complete Reference

This is the full specification of the existing C game in
[`../Breakout_C`](../../Breakout_C). It documents the code **as it is today**,
after the refactor that introduced the `AppContext` struct and split the old
monolithic `screens.c` into focused modules. (The older `Breakout_C/docs/*.md`
describe a *pre-refactor* codebase that no longer exists — see
[§ Already-resolved issues](#already-resolved-issues).)

Every subsystem below follows the same template:

> **What it does** · **Key data** · **Behaviour / flow** · **Quirks** ·
> **C → C++ mapping**

The C++ mapping is the point of this whole document — it's the bridge from "how
the C does it" to "how idiomatic C++ would."

---

## Contents

1. [At a glance](#at-a-glance)
2. [How the C version builds & runs](#how-the-c-version-builds--runs)
3. [Runtime architecture](#runtime-architecture)
4. [Subsystem reference](#subsystem-reference)
   - [4.1 App lifecycle & initialisation](#41-app-lifecycle--initialisation)
   - [4.2 Rendering & the RenderContext](#42-rendering--the-rendercontext)
   - [4.3 Text rendering](#43-text-rendering)
   - [4.4 Buttons](#44-buttons)
   - [4.5 Game simulation: paddle, ball, bricks, collision](#45-game-simulation-paddle-ball-bricks-collision)
   - [4.6 The game loop & timing](#46-the-game-loop--timing)
   - [4.7 Input: keyboard name entry](#47-input-keyboard-name-entry)
   - [4.8 Screens & UI](#48-screens--ui)
   - [4.9 Save system](#49-save-system)
   - [4.10 High scores & hashing](#410-high-scores--hashing)
   - [4.11 Audio & the music state machine](#411-audio--the-music-state-machine)
   - [4.12 Settings & auto-play](#412-settings--auto-play)
5. [Data & file formats](#data--file-formats)
6. [Constants & tunables](#constants--tunables)
7. [Catalogue of quirks, dead code & bugs](#catalogue-of-quirks-dead-code--bugs)
8. [C → C++ idiom map (summary)](#c--c-idiom-map-summary)
9. [Already-resolved issues](#already-resolved-issues)

---

## At a glance

| | |
|---|---|
| Language / libs | C99, SDL2 + SDL2_image + SDL2_ttf + SDL2_mixer |
| Lines of code | ~3,265 across `src/` |
| Window | 864 × 558, accelerated renderer with vsync |
| State model | Single `AppContext` passed by pointer everywhere |
| Screens | 10 (+1 terminal "quit confirmed" state) |
| Persistence | 3 plain-text files in `resources/game_load_files/` |
| Build | `make` (uses `pkg-config` to find SDL2) |
| Tests | Unity framework, `scores` module only (12 tests) |

### Module map

```
src/
├── core/
│   ├── main.c          (433) init, media load, audio, cleanup, main()
│   ├── main.h
│   ├── app_context.h   (50)  RenderContext / AudioContext / GameContext /
│   │                          InputContext / AppContext  ← the spine
│   ├── constants.h     (177) every tunable, every enum, core structs
│   └── header.h        (18)  the common include block
├── screens.c           (54)  handle_screen_state() — the dispatch switch
├── screens.h
├── game.c              (418) bricks, physics, collision, render, game loop
├── game.h
├── input.c             (254) on-screen keyboard for name entry
├── input.h
├── save_system.c       (223) slot save/load + savefile.txt I/O
├── save_system.h
├── scores.c            (495) high-score entries, DJB2 hash, file I/O
├── scores.h
├── text.c              (151) TTF text render, sizing, word-wrap
├── text.h
├── ui_screens.c        (715) the 7 menu/overlay screens
├── ui_screens.h
└── components/buttons/
    ├── buttons.c       (147) create/render/destroy/hit-test buttons
    └── buttons.h
```

### Dependency direction

```
main.c ─► everything (owns lifecycle)
screens.c ─► game.c, input.c, ui_screens.c        (dispatch only)
ui_screens.c ─► buttons, game, input, save_system, scores, text
game.c ─► save_system, text, (main.c for play_music)
input.c ─► buttons, text
save_system.c ─► game.c (set_default_state, init_bricks)
buttons.c ─► text
text.c ─► (leaf)
scores.c ─► (leaf, no SDL — that's why it's the only testable module)
```

`scores.c` is deliberately SDL-free, which is exactly why it's the one module
with unit tests. Keep that property in C++ — pure logic separated from I/O is
testable logic.

---

## How the C version builds & runs

```bash
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer   # deps
make            # → build/main (pkg-config finds SDL2)
make run        # build + run from project root
make test       # build + run the Unity tests for scores
make clean
```

The binary **must be run from the project root** because every resource path is
relative (`resources/...`). There is no asset-path abstraction — paths are
`#define`d string literals at the top of `main.c` and hard-coded inside the save
/ score / settings functions.

**C++ takeaway:** centralise asset paths (a `Paths` struct or constants header),
and consider resolving them relative to the executable so the game runs from
anywhere.

---

## Runtime architecture

### The one struct to rule them all: `AppContext`

All mutable state lives in a single `AppContext` (`app_context.h`), composed of
four sub-contexts plus a few loose fields. A pointer to it is threaded through
essentially every function.

```c
typedef struct {
    RenderContext render;   // window, renderer, textures, font
    AudioContext  audio;    // 4 Mix_Music* + MusicState
    GameContext   game;     // active game, 3 save slots, walls, selected_slot
    Settings      settings; // auto_play flag
    ScoreContext  scores;   // 3 high-score entries
    ScreenState   current_screen;
    ScreenState   previous_screen;
    InputContext  input;    // player_name[512]
    int           intense_music_active;
} AppContext;
```

| Sub-context | Fields | Notes |
|---|---|---|
| `RenderContext` | `window`, `renderer`, `background_texture`, `button_texture`, `save_button_texture`, `font` | All raw SDL pointers, manually destroyed in `close_app`. |
| `AudioContext` | `lobby_music`, `intense_music`, `normal_music`, `badumtss_sound`, `state` | `state` is `{int is_playing; MusicTrack current_track;}`. |
| `GameContext` | `active_game`, `saved_games[3]`, `selected_slot`, 4 wall `SDL_Rect`s | `selected_slot` 0 = "quick game, no slot", 1–3 = a save slot. |
| `InputContext` | `player_name[512]` | The on-screen-keyboard buffer. |

**C++ mapping.** `AppContext` is a *god-struct* — convenient, but it means any
function can touch any state. In C++ this naturally decomposes into objects with
ownership and narrow interfaces: a `Renderer`/`Window` RAII wrapper, an
`AudioPlayer`, a `Game`/`World`, a `SaveManager`, a `HighScoreTable`. The "thread
a pointer everywhere" pattern becomes "each system owns its data and exposes
methods." You don't have to do this all at once — Doc 03 introduces the split
gradually.

### Control flow

```
main()                      (main.c, #ifdef picks main vs WinMain)
 └─ main_loop()
     ├─ init_app(&app)      create window/renderer, init SDL subsystems
     ├─ load_media(&app)    textures, font, music, + load save/scores/settings
     └─ while (current_screen != SCREEN_QUIT_CONFIRMED)
            handle_screen_state(&app)     ← the heartbeat
        close_app(&app)     save everything, free everything, SDL_Quit
```

`handle_screen_state()` (`screens.c`) is a single `switch` on
`app->current_screen`. Each case calls **one** screen function, which runs its
**own** blocking event loop until the user does something that changes
`current_screen`, then returns. Control bubbles back to `main_loop`, which calls
`handle_screen_state` again for the new screen.

> This iterative, return-to-the-top design is the **fix** for the old recursive
> state machine that used to overflow the stack. Preserve this shape in C++.

### The screen state machine

```c
typedef enum {
    SCREEN_MAIN_MENU, SCREEN_GAME, SCREEN_LOAD_GAME, SCREEN_QUIT_MENU,
    SCREEN_GAME_OVER, SCREEN_HIGH_SCORE, SCREEN_SETTINGS, SCREEN_SAVE_GAME,
    SCREEN_KEYBOARD, SCREEN_KEYBOARD_QUICK,
    SCREEN_QUIT_CONFIRMED   // terminal — exits main_loop
} ScreenState;
```

Transition map (→ = "sets current_screen to"):

```
MAIN_MENU ──"Quick Game"──► GAME            (selected_slot stays 0)
          ──"Load Game"───► LOAD_GAME
          ──"High Score"──► HIGH_SCORE
          ──"Settings"────► SETTINGS
          ──ESC/✕─────────► QUIT_MENU

LOAD_GAME ──slot (empty)──► KEYBOARD        (enter a name first)
          ──slot (in use)─► GAME
          ──"Back"────────► MAIN_MENU

GAME ──SPACE──► (in-progress)  ──ESC──► QUIT_MENU (saves first)
     ──ball lost / all bricks──► GAME_OVER (via 2s delay, status=LOST)

QUIT_MENU ──"Yes"──────► QUIT_CONFIRMED (exit)
          ──"Go Back"──► previous_screen
          ──"Save Game"► SAVE_GAME        (only if came from GAME, slot 0)

GAME_OVER ──"Main Menu"──► MAIN_MENU
          ──"High Score"─► HIGH_SCORE
SETTINGS  ──toggle/reset─► SETTINGS (re-enters itself)
          ──"Back"───────► MAIN_MENU
SAVE_GAME ──slot──► (inline name entry) ──► MAIN_MENU
KEYBOARD  ──Save/Enter──► GAME or MAIN_MENU
```

Note `SCREEN_KEYBOARD_QUICK` is **defined but never dispatched** — the quick
name-entry path is called inline from the save screen instead. See the
[quirks catalogue](#catalogue-of-quirks-dead-code--bugs).

**C++ mapping.** The enum + `switch` is fine and idiomatic; `enum class
ScreenState` plus a `switch` (or a `std::unordered_map<ScreenState,
std::function<...>>`, or polymorphic `Screen` objects with a `run()` method) all
work. A clean OO version: an abstract `Screen` base with `Screen* run()`
returning the next screen — but the plain enum-switch is perfectly good and
easier to follow. Don't over-engineer this.

### Two kinds of loop

The codebase has **two distinct loop styles**, and telling them apart matters:

1. **Menu/overlay loops** (every function in `ui_screens.c`, plus the keyboard
   in `input.c`): create buttons → render once → `while (!quit)` polling events,
   only reacting to clicks/keys, **not redrawing each frame** → on transition,
   destroy buttons and return. These are event-driven and mostly idle.
2. **The game loop** (`run_game_loop` in `game.c`): the real-time loop —
   continuously `update_game` + throttled `render_game`. This is the only place
   with frame timing.

**C++ mapping.** The menu loops repeat the same skeleton ~7 times (build
buttons, poll, dispatch on `strcmp(button.text, ...)`, cleanup). That
duplication is begging for a reusable `Menu` class or a small helper that takes
a list of `{Button, callback}` and runs the loop. The string-compare dispatch
(`strcmp(clicked->text, "Quick Game")`) becomes a `std::function` per button or
an enum action id — no more matching on display text.

---

## Subsystem reference

### 4.1 App lifecycle & initialisation

**What it does.** `main.c` owns the program: bring SDL up, load all media, run
the screen loop, tear everything down.

**Key data.** The whole `AppContext` (zeroed with `memset` at start of
`init_app`).

**Behaviour / flow.**
- `init_app`: `SDL_Init(VIDEO|AUDIO)` → create window (`864×558`,
  `SDL_WINDOW_SHOWN`) → create renderer (`ACCELERATED | PRESENTVSYNC`) → enable
  alpha blending → `IMG_Init(PNG)` → `Mix_OpenAudio(44100, default, 2, 2048)` →
  `TTF_Init()`. Then seed initial state: `current_screen = MAIN_MENU`,
  `selected_slot = 0`, music = lobby/not-playing. Returns `0` on any failure.
- `load_texture`: `IMG_Load` → `SDL_CreateTextureFromSurface` → free the
  surface. Returns the texture (caller owns it).
- `load_media`: loads 3 textures + 1 font + 4 music tracks, then calls
  `load_save_file`, `load_entries`, `load_settings`. Any failure → return 0.
- `close_app`: **saves** scores + game state + settings to disk, then destroys
  every texture, frees every `Mix_Music`, frees every heap string (entry
  names/hashes, saved-game names, active-game name), closes the font, destroys
  renderer + window, quits all SDL subsystems. Null-guards and re-nulls each
  pointer. This is thorough — a good model.

**Quirks.**
- The window is created titled `"Background Display"`; the real title is set
  per-screen via `SDL_SetWindowTitle`.
- `init_app` does `memset(app, 0, sizeof(*app))` — relies on all-zero being a
  valid starting state (it is, here).

**C → C++ mapping.**
- Each `SDL_*` create/destroy pair is a textbook **RAII** case. Wrap
  `SDL_Window`, `SDL_Renderer`, `TTF_Font`, `Mix_Music`, `SDL_Texture` in
  `std::unique_ptr<T, custom_deleter>` (or hand-written RAII classes). Then
  `close_app`'s 90 lines of manual teardown become **destructors that run
  automatically** — and in the correct reverse order.
- The "return 0 on failure" C idiom becomes either exceptions (throw on init
  failure) or `std::optional`/`expected`. For init code, exceptions read
  cleanest.
- `init_app` + `load_media` + `close_app` → a constructor/destructor of an
  `Application` (or `Game`) class. SDL subsystem init/quit is itself a great RAII
  lesson: an `SDLContext` object whose ctor calls `SDL_Init` and whose dtor calls
  `SDL_Quit`.

---

### 4.2 Rendering & the RenderContext

**What it does.** Holds the renderer + shared textures + font, and is passed to
everything that draws.

**Key data.** `RenderContext { window, renderer, background_texture,
button_texture, save_button_texture, font }`.

**Behaviour.** Drawing is immediate-mode SDL: `SDL_RenderClear` →
`SDL_RenderCopy(background)` → draw widgets/text/rects → `SDL_RenderPresent`.
Solid shapes (paddle, ball, walls, bricks) are `SDL_SetRenderDrawColor` +
`SDL_RenderFillRect`. The background is a full-window PNG copied first each
frame.

**Quirks.** Draw colour is set redundantly before each wall fill (5× the same
`0,0,0,255`). Bricks set white once then override per-brick. Harmless, just
noise.

**C → C++ mapping.** A thin `Renderer` class that owns the `SDL_Renderer*` and
exposes `clear()`, `present()`, `draw_rect(rect, color)`, `draw_texture(tex,
dst)`. Color becomes a small `struct Color { Uint8 r,g,b,a; }` (or reuse
`SDL_Color`). The "pass `RenderContext*` to every draw function" pattern becomes
"draw functions are methods on `Renderer`, or take `Renderer&`."

---

### 4.3 Text rendering

**What it does.** Render a string with the TTF font at a given size/position,
measure text, center it, and word-wrap long lines.

**Key data.** Uses `render->font` (a single shared `TTF_Font*`, resized on the
fly).

**Behaviour.**
- `render_text(render, text, x, y, font_size)`: `TTF_SetFontSize` →
  `TTF_RenderText_Solid` (black) → texture → `SDL_RenderCopy` → **destroys the
  texture and frees the surface** (no leak — this was a fixed bug).
- `get_font_data`: `TTF_SetFontSize` + `TTF_SizeText` to get width/height
  without drawing. **Calls `exit(1)` if the font is null** (a hard abort buried
  in a helper — see quirks).
- `print_centered_text`: measures; if width ≤ half-screen, centers on one line;
  otherwise calls `render_wrapped_text`.
- `render_wrapped_text`: greedy word-wrap — accumulate words into a line, when
  the next word would exceed `SCREEN_WIDTH-40` flush the line and start a new
  one. Uses `sscanf("%255s%n")` to walk words.

**Quirks.**
- One shared font whose size is mutated by `TTF_SetFontSize` on every call —
  there's global-ish state in the font object.
- `get_font_data` aborts the whole process (`exit(1)`) on a null font. A drawing
  helper should not own the kill switch.
- The single solid colour (black) is hard-coded; there's no colored text.

**C → C++ mapping.**
- This is a natural `TextRenderer` / `Font` class. Wrap `TTF_Font*` in RAII.
- Replace `exit(1)` with an exception or an error return — library code
  shouldn't unilaterally terminate.
- `TTF_RenderText_Solid` produces a fresh surface+texture **every call**. In C++
  you'd likely add a small **glyph/string texture cache** (`std::unordered_map<
  std::string, Texture>`) — the HUD score re-renders every frame today.
- The manual word-buffer juggling (`char line[512]`, `char word[256]`,
  `snprintf` into temporaries) becomes `std::string` + `std::istringstream` /
  `std::vector<std::string>` words. Much shorter, no buffer-size constants.

---

### 4.4 Buttons

**What it does.** A reusable clickable widget: a rect + a text label + an
optional `on_click` function pointer.

**Key data.**
```c
typedef struct Button {
    SDL_Rect rect;
    char *text;                       // heap copy of the label
    void (*on_click)(struct Button *);// function pointer (mostly unused!)
} Button;
```

**Behaviour.**
- `create_button(x,y,w,h,text,handler)`: `malloc` the button, `malloc`+copy the
  text, store handler. Returns `NULL` on allocation failure.
- `render_button`: copy `button_texture` into the rect, then draw the label
  centered.
- `render_save_button`: a *fancier* renderer used for save slots — draws the
  `save_button_texture` and overlays slot status ("Empty" / "Saved" + name +
  score / "Lost") with several hand-tuned position multipliers (`*0.1`, `*0.8`,
  `*1.2`).
- `is_point_in_rect` + `find_clicked_button`: hit-testing; returns the first
  button whose rect contains the point.
- `add_button`: append to a fixed `Button*[10]` array, bounds-checked at 10.
- `destroy_button`: free text + free button.

**Quirks.**
- The `on_click` function pointer is part of the type but **almost never used** —
  every screen passes `NULL` and instead dispatches by **comparing the button's
  display text** (`strcmp(clicked->text, "Quick Game")`). So the click behaviour
  is coupled to the human-readable label.
- Buttons are stored in raw fixed arrays (`Button *buttons_main[10]`) with a
  manual `num_buttons` counter, created and destroyed in every screen function.
- `render_save_button`'s magic multipliers are pure trial-and-error layout.

**C → C++ mapping.**
- `Button` becomes a value type (`struct Button { SDL_Rect rect; std::string
  text; std::function<void()> on_click; };`) — no `malloc`/`free`, no manual
  text copy. Store them in `std::vector<Button>`; the fixed `[10]` and
  `num_buttons` disappear.
- Make `on_click` actually carry the behaviour (a lambda capturing what it
  needs), eliminating the `strcmp`-on-label dispatch. This is one of the most
  satisfying single upgrades — labels become *display only*.
- `find_clicked_button` → `std::find_if(buttons.begin(), buttons.end(), …)`.
- `render_save_button` overload → either a `Button` subtype or a `draw` that
  takes the slot's `GameState`. Replace magic multipliers with named layout
  constants while you're there (a reproduce-vs-tidy call).

---

### 4.5 Game simulation: paddle, ball, bricks, collision

This is the heart. All in `game.c`.

**What it does.** Set up the field, move the paddle from keyboard, move the ball,
bounce it off walls / bricks / paddle, age bricks, score, detect end-of-game.

**Key data** (inside `GameState`):
```c
char *name;
SDL_Rect paddle_bottom;          // the paddle
SDL_Rect ball;
int ball_speed_x, ball_speed_y;  // velocity components (start 3,3)
int game_paused;                 // 1 = round is over (win OR lose)
int score;
int temp_collision;              // DEAD — saved/loaded but never read
int status;                      // GAME_STATUS_EMPTY/SAVED/LOST (0/1/-1)
Brick bricks[5][9];              // Brick = { SDL_Rect rect; int health; }
```
Plus four wall rects in `GameContext` (`left/right/top/bottom_wall`).

**Field setup.**
- `init_bricks(app, use_saved)`: lays out a 5×9 grid, centered horizontally,
  starting at `y=60`, each brick `70×25` with `8px` gaps. Fresh game → row *i*
  gets health `5 - i` (top row 5, bottom row 1). Saved game → keep stored
  health.
- `set_default_state`: paddle centered at `0.83·H`, ball centered, velocity
  `(3,3)`, score 0, `status = EMPTY`, all bricks reset to `5 - i`.
- `init_game`: `load_game_state` (decide fresh vs. slot vs. continue) then build
  the four walls.

**Per-frame update (`update_game`).**
1. **Bail if the window is minimised or unfocused** (`is_minimized` /
   `is_focused` via `SDL_GetWindowFlags`).
2. If `score > 20` and intense music not yet active → switch to intense track
   (one-shot).
3. If `status == EMPTY` → promote to `SAVED`.
4. Read live keyboard (`SDL_GetKeyboardState`):
   - Left/Right move the paddle by `PADDLE_SPEED` (5), bounded by
     `PADDLE_BOUNDARY_PADDING_X` (70) on each side.
   - Holding **Left Alt** applies the move **twice** → double speed.
5. **Move the ball by subtraction:** `ball.x -= ball_speed_x; ball.y -=
   ball_speed_y;`. With initial `(3,3)` the ball travels **up-and-left** first.
6. **Collisions** (all via `SDL_HasIntersection`):
   - left/right wall → flip `ball_speed_x`.
   - top wall → flip `ball_speed_y`.
   - each live brick (health ≠ 0) that intersects: `health--`, flip
     `ball_speed_y`, nudge `ball.y -= ball_speed_y - 5` (anti-stick), `score++`.
   - paddle → flip `ball_speed_y`, nudge `ball.y -= ball_speed_y + 5`.
   - bottom wall → `game_paused = 1` (round over).
   - track `all_blocks_destroyed`; if every brick health ≤ 0 → `game_paused = 1`.
7. **Auto-play** (if enabled): snap paddle x to follow the ball (the clamp that
   should keep it on screen is **dead code** — see quirks).
8. If `game_paused` (the `else` branch): `SDL_Delay(2000)` (a hard 2-second
   freeze), `status = LOST`, `save_game_state`, go to `SCREEN_GAME_OVER`.

**Rendering (`render_game`).** Clear → background → score HUD (`[Score: N]`) →
fill paddle + 4 walls (black) → fill ball → fill each live brick in its
health-shade of grey → present. Brick colour by health: 1→20% black, 2→40%,
3→60%, 4→80%, 5→100% (so tougher = darker).

**Quirks** (big ones — these define the game's "feel"):
- **Win and lose are the same.** Clearing all bricks sets `game_paused = 1`,
  which routes through the identical `else` branch as dropping the ball:
  2-second freeze → `status = LOST` → game-over screen. **There is no win
  screen, and a victory is recorded as a loss.**
- **The Alt double-speed** is a feature-by-accident: the move is applied once
  inside the `if (LALT)` and once unconditionally.
- **Auto-play can leave the screen:** `paddle.x` is assigned the unclamped
  target *before* the bounds check runs, and the bounds check only edits a local
  variable that's then discarded. The clamp does nothing.
- **`SDL_Delay(2000)` blocks the entire loop** — the window is frozen/unresponsive
  for 2s on game over.
- Ball moves by **subtraction** of velocity, which is unusual but self-consistent.
- Multiple bricks hit in one frame **each** flip `ball_speed_y` — an even number
  of hits cancels out.
- `temp_collision` is written to save files but never read.

**C → C++ mapping.**
- `Brick bricks[5][9]` → `std::array<std::array<Brick,9>,5>` (or a flat
  `std::vector<Brick>` + a small grid view). Bounds-checked iteration, range-for.
- `int status` magic numbers → `enum class GameStatus { Empty, Saved, Lost };`
  And add a real `Won` state if you decide to *fix* the win==lose quirk.
- `int game_paused` (really "round over") → rename to a clear flag or fold into
  `GameStatus`.
- `SDL_Rect ball/paddle` → keep `SDL_Rect` for rendering, but consider a
  `Vec2`/float position for physics so movement isn't integer-locked (a *fix*,
  optional).
- Collision: `SDL_HasIntersection` is fine to keep; or write a tiny
  `bool intersects(const SDL_Rect&, const SDL_Rect&)` to make the code
  self-contained and testable without SDL.
- The whole `update_game` is a method `World::update(dt)`; `render_game` is
  `World::render(Renderer&)`. Input reading splits out into an `Input` poll.
- The `SDL_Delay(2000)` and win==lose are flagged in Doc 03 as explicit
  reproduce-vs-fix decisions.

---

### 4.6 The game loop & timing

**What it does.** The real-time loop for `SCREEN_GAME` (`run_game_loop`).

**Behaviour.**
- Start normal-game music; draw the "Press [SPACE] to Start" instruction panel;
  `init_game`. `game_in_progress = 0` until SPACE.
- Loop: poll events (ESC/✕ → save + go to `QUIT_MENU`; SPACE → start). When in
  progress: compute `elapsed = now - last_render`, call `update_game`
  **every iteration**, and call `render_game` **only when** `elapsed ≥ 1000/60`.
  Exit the loop if `current_screen` changed away from `GAME`.

**Quirks.**
- **The simulation is not on a fixed timestep.** `update_game` is called once
  per spin of the `while` loop, while only `render_game` is time-gated (it uses
  `last_render_time`; update has no equivalent throttle). So the simulation
  *step-rate is coupled to the loop/frame/CPU rate rather than to wall-clock* —
  the classic "physics tied to frame rate" trap. (This is a structural property
  of the loop; the fix below holds regardless of how fast any given machine
  actually runs it.)
- Frame timing uses integer `1000 / TARGET_FPS` (= 16) — fine here since
  `TARGET_FPS` is 60, but it *was* `1000/2000 = 0` in the pre-refactor code.

**C → C++ mapping.** The canonical fix is a **fixed-timestep loop**: accumulate
real elapsed time and step the simulation in fixed `dt` chunks, render the latest
state (optionally interpolated). This is a great, self-contained C++ lesson and
makes the game speed deterministic across machines. `SDL_GetTicks()` →
`std::chrono::steady_clock` if you want to go full-STL, or keep `SDL_GetTicks`
for simplicity.

---

### 4.7 Input: keyboard name entry

**What it does.** An on-screen keyboard screen to type a player name (≤10 chars)
using the physical keyboard, with a visible underline and a "Save Name" button.

**Key data.** `app->input.player_name[512]`, the slot being saved
(`game.selected_slot`).

**Behaviour.**
- `run_keyboard_loop(app, return_name)` is the unified implementation, called two
  ways:
  - `run_keyboard_input(app)` → `return_name = 0`: used by `SCREEN_KEYBOARD`
    (naming a save slot). On finish it writes the name into the slot and goes to
    `SCREEN_GAME`.
  - `run_quick_name_input(app)` → `return_name = 1`: returns the typed string to
    the caller (used inline by the save-game screen).
- `update_keyboard`: scans scancodes A–Z; Shift selects lowercase (note: default
  is **uppercase**); enforces the 10-char limit; Backspace deletes; Enter
  finalises. Key repeat is rate-limited to one accepted key per
  `KEYPRESS_DELAY_MS` (200ms).
- `finalize_keyboard_input`: empty name defaults to `"Empty"`; then either
  returns the name or commits it to the slot.
- Buttons are created and `destroy_button`'d around the loop (no leak).

**Quirks.**
- Lots of `printf` debug spam to stdout on every keypress ("X is pressed",
  "Name is …").
- The 200ms hand-rolled repeat delay is a workaround for reading key *state*
  (held) rather than key *events*. SDL has proper text input
  (`SDL_StartTextInput` + `SDL_TEXTINPUT` events) that handles repeat, layout,
  and non-Latin input for free — unused here.
- This `run_keyboard_loop` already **unified** what used to be two ~80%-duplicate
  functions (`MainKeyboardLoop` / `QuickGameKeyboardLoop`). Good.

**C → C++ mapping.**
- `player_name` → `std::string` (push_back / pop_back instead of manual index +
  `'\0'`).
- Strongly consider SDL's `SDL_TEXTINPUT` event path in C++ — it's the *correct*
  way to do text entry and removes the scancode-scanning + delay hack. (A
  behaviour change, so flag it as reproduce-vs-fix.)
- Drop the `printf` spam or route it through a `Log` with levels.

---

### 4.8 Screens & UI

`ui_screens.c` holds seven screens; each is a self-contained
build-buttons → render → event-loop → cleanup function. Summary:

| Screen | Buttons | Key behaviour |
|---|---|---|
| `render_main_menu` | Quick Game, Load Game, High Score, Settings | Quick Game resets state + `selected_slot` stays 0; ESC → quit menu. |
| `render_load_game_screen` | Back + 3 save slots | Empty slot → keyboard (name first); used slot → game. Shows slot status via `render_save_button`. |
| `render_quit_menu` | Yes, Go Back, [Save Game] | "Save Game" only appears if we came from `GAME` with `selected_slot == 0`. "Go Back" returns to `previous_screen`. |
| `render_game_over` | Main Menu, High Score | Plays the "ba-dum-tss". If `selected_slot` set → "Game Saved" + records a high-score `Entry`; else "Game Not Saved". Then resets `selected_slot = 0`. |
| `render_high_scores` | Back | Lists the 3 entries `name : score`. Return target depends on `previous_screen` (game-over vs main menu). |
| `render_settings` | Back, Enable/Disable, Reset Save File | Toggles `auto_play` (re-enters Settings to redraw the label); "Reset Save File" marks all slots `LOST`, writes, reloads. |
| `render_save_game_screen` | Back + 3 slots | Pick a slot → inline `run_quick_name_input` → copy `active_game` into the slot → main menu. |

**Quirks.**
- Dispatch is by `strcmp` on the button label everywhere (incl. `sscanf(text,
  "Slot %d", &n)` to recover the slot number from the label text — parsing the
  UI string to get data).
- Settings toggling works by *changing the button's label text* (Enable ⇄
  Disable) and re-entering the screen, rather than holding boolean UI state.
- `render_game_over` both *renders* and has the side effect of *recording the
  high score* and *freeing* the temp entry — view and model logic interleaved.
- High-score "back" target is derived from `previous_screen` with a small
  `switch`.

**C → C++ mapping.**
- The seven near-identical skeletons → one `Menu`/`Screen` abstraction (a list of
  buttons + a render fn + an event loop), removing ~500 lines of repetition.
- Replace `strcmp`/`sscanf`-on-label dispatch with per-button callbacks
  (`std::function`) or an action enum, and store the slot index *as data* on the
  button, not parsed back out of its label.
- Separate "draw the game-over screen" from "commit the score" — the score
  recording belongs in the model/`HighScoreTable`, triggered on the
  game-over *transition*, not inside the render function.

---

### 4.9 Save system

**What it does.** Three save slots persisted to one text file, plus the in-memory
"which slot / fresh / continue" logic.

**Key data.** `game.saved_games[3]` (array of `GameState`), `game.selected_slot`
(0 = none/quick, 1–3 = slot), and `active_game`.

**Behaviour.**
- `load_game_state` (called by `init_game`): decides what `active_game` becomes —
  - slot chosen & slot is `LOST` → reset to default + fresh bricks, copy back to
    slot;
  - slot chosen & slot in use → copy slot into `active_game`, keep its bricks;
  - no slot but `active_game.status == SAVED` → continue current bricks;
  - otherwise → reset to default + fresh bricks.
- `save_game_state`: promote `EMPTY`→`SAVED`; if a slot is selected, copy
  `active_game` into it (in memory).
- `save_game_state_to_file` / `load_save_file`: serialise all 3 slots to/from
  `resources/game_load_files/savefile.txt`. One line per slot: name + 14 ints +
  45 brick-health ints. On a malformed line, reset that slot to default + mark
  `LOST` + name `"Empty"`.
- `print_slot_file`: debug dump of a slot to stdout.

**Quirks.**
- The whole format is **space-separated on one line** (see
  [Data formats](#data--file-formats)) — so a player name containing a space
  would break parsing. (The high-score path defends against this with
  `replace_spaces_with_underscores`; the *save* path does not.)
- `GameState` is copied by value (`a = b`) including its `char *name` pointer —
  meaning two `GameState`s can share/alias the same heap `name`, and assignments
  can leak or double-reference names. The code mostly dodges this by `strdup`/
  `free` discipline, but it's fragile (a classic shallow-copy hazard).
- `temp_collision` is serialised but meaningless.

**C → C++ mapping.**
- This is the **poster child for `std::string` + RAII**: once `name` is a
  `std::string`, value-copying a `GameState` *just works* — deep copy, no
  aliasing, no leaks, no `strdup`. The entire class of bugs above evaporates.
- File I/O: `std::ifstream`/`std::ofstream` with `operator>>`. Consider a less
  fragile format (one field per line, or JSON via a header-only lib) — but the
  simplest faithful port keeps the space-separated layout.
- The fresh/continue/load decision tree is pure logic → put it on a
  `SaveManager` and unit-test it (like `scores`).

---

### 4.10 High scores & hashing

**What it does.** Keep a top-3 table of `{name, score, timestamp, hash}`, persist
it, verify integrity with a hash, and merge new scores in.

**Key data.**
```c
typedef struct { char *name; int score; time_t timestamp; char *hash; } Entry;
typedef struct { Entry entries[NUM_SCORES]; } ScoreContext;   // NUM_SCORES = 3
```

**Behaviour.**
- `djb2_hash` / `compute_hash`: a **DJB2** hash over name+score+timestamp,
  formatted as a 16-char hex string. (This *replaced* the original OpenSSL SHA-256
  dependency — good call.)
- `create_entry(name, score)`: spaces→underscores, timestamp = `time(NULL)`,
  compute hash. (On allocation failure it `exit(EXIT_FAILURE)` — another buried
  hard-abort.)
- `load_entries`: read up to 3 lines, parse `name score timestamp hash`,
  recompute the hash, and **reject the row if the stored hash doesn't match**.
  So hand-editing `highscores.txt` silently drops edited rows.
- `update_high_scores`: deep-copy the 3 entries + the new one into a temp array
  of 4, `qsort` descending by score, keep the top 3.
- `save_entries_to_file`: `qsort` descending, write non-null entries.
- `compare_scores`: `qsort` comparator, descending by score.
- **Dead code:** `load_records`, `load_dummy_entries`, `swap_entries`,
  `free_entry_hash` are defined/declared but never called by the game.

**Quirks.**
- Hash verification is **active now** (the old docs said it was commented out).
  Net effect: a cheap tamper check that makes manual edits "fail" rather than
  load.
- `Entry` mixes ownership (`char *name`, `char *hash` both heap) with values
  (`int`, `time_t`) — every create/copy/free path has to track two strings.
- `time_t` is printed/scanned as `long` — portable-ish but assumes a range.

**C → C++ mapping.**
- `Entry` → `struct Entry { std::string name, hash; int score;
  std::time_t timestamp; };` — copy/move just work; `update_high_scores`'s
  deep-copy dance becomes a `std::vector<Entry>`, `std::sort` with a lambda
  (`a.score > b.score`), `resize(3)`.
- `qsort` (void*, comparator returning int) → `std::sort` + comparator lambda —
  type-safe and faster to write.
- Keep the SDL-free property: `HighScoreTable` stays a pure, unit-tested class.
- The 12 existing Unity tests map almost 1:1 onto a C++ test framework
  (Catch2/doctest/GoogleTest) — a nice early win for "tests carry over."

---

### 4.11 Audio & the music state machine

**What it does.** Four tracks — lobby menu music, "normal" in-game, "intense"
in-game (after score > 20), and a one-shot "ba-dum-tss" sting on game over.

**Key data.** `AudioContext { lobby_music, intense_music, normal_music,
badumtss_sound, state }` where `state = { int is_playing; MusicTrack
current_track; }`.

**Behaviour.**
- `play_music(audio, track)`: a small state machine on `is_playing`
  (`0` stopped / `1` playing / `2` paused):
  - already playing & same track → no-op;
  - already playing & different track → switch (lobby & intense use
    `Mix_FadeInMusic`; normal & badumtss use `Mix_PlayMusic`);
  - stopped → `Mix_PlayMusic`;
  - paused → `Mix_ResumeMusic`.
- `pause_music`: if playing, pause and set `is_playing = 2`.
- Track selection is driven by `handle_screen_state` (lobby on menus, normal on
  game) and by `update_game` (intense once `score > 20`) and
  `render_game_over` (badumtss).

**Quirks.**
- `is_playing` is a tri-state `int` (0/1/2) with meaning only in comments.
- Fade vs. instant play is inconsistent between branches (fade only on the
  "switch while playing" path, and only for two of the tracks).
- The intense-music switch is one-way (`intense_music_active` latches); it never
  goes back to normal if the score logic changed.

**C → C++ mapping.**
- `int is_playing` (0/1/2) → `enum class PlaybackState { Stopped, Playing,
  Paused };`; `MusicTrack` → `enum class`.
- Wrap `Mix_Music*` in RAII; an `AudioPlayer` class owns the four tracks and
  exposes `play(Track)`, `pause()`, `resume()`. The branchy `play_music` becomes
  a clean method, and the fade/instant choice can be a per-track property.

---

### 4.12 Settings & auto-play

**What it does.** One persisted boolean, `auto_play`, that makes the paddle track
the ball automatically (a demo/attract mode).

**Key data.** `Settings { int auto_play; }`, file
`resources/game_load_files/settings.txt` (a single `0` or `1`).

**Behaviour.** `load_settings`/`save_settings_to_file` read/write the one int.
The settings screen toggles it; `update_game` reads it to drive the paddle.

**Quirks.** As noted in §4.5, the auto-play paddle-bounds clamp is dead, so in
auto mode the paddle can run past the screen edge.

**C → C++ mapping.** `struct Settings { bool auto_play; };`, loaded/saved via
`std::fstream`. Trivial — but a good spot to introduce a tiny typed
config-file helper you can extend later (volume, difficulty, …).

---

## Data & file formats

All under `resources/game_load_files/`.

### `savefile.txt` — 3 lines, one per slot
```
<name> <pad.x> <pad.y> <pad.w> <pad.h> <ball.x> <ball.y> <ball.w> <ball.h>
       <spd.x> <spd.y> <paused> <score> <temp_collision> <status>
       <45 brick-health ints, row-major 5×9>
```
(all on one physical line). Example:
```
DD 382 463 100 20 422 269 20 20 3 3 0 0 0 0 5 5 5 5 5 5 5 5 5 4 4 4 …(45 total)
```
`%511s` name + 14 ints parsed by one `fscanf`; 45 brick ints by a nested loop.
Malformed line → slot reset to default + `LOST` + name `"Empty"`.

### `highscores.txt` — up to 3 lines
```
<name> <score> <unix_timestamp> <16-hex-char DJB2 hash>
```
Example:
```
Dummy 135 1712055060 0cf0a61ab8c0a385ee77e18cf2f372141f1673523088e876fddc1b1aa00256f3
```
(The example file ships 64-char SHA-256 hashes from the old OpenSSL version;
the current DJB2 verifier produces 16-char hashes and will reject these legacy
rows — a real, observable consequence of the hash change.)

### `settings.txt` — 1 line
```
0        # or 1 — the auto_play flag
```

### `quotes.txt` — **unused**
Contains the two menu taglines, but the game reads them from the hard-coded
`GAME_QUOTE_PRIMARY` / `GAME_QUOTE_SECONDARY` constants instead. The file is dead
data left over from an earlier design.

---

## Constants & tunables

Everything lives in `constants.h`. Highlights worth knowing when re-tuning:

| Constant | Value | Meaning |
|---|---|---|
| `SCREEN_WIDTH` / `SCREEN_HEIGHT` | 864 / 558 | Window size |
| `TARGET_FPS` | 60 | Render cap (`1000/60 ≈ 16ms`) |
| `NUM_BRICK_ROWS` / `NUM_BRICK_COLS` | 5 / 9 | Brick grid (45 bricks) |
| `BRICK_WIDTH/HEIGHT` | 70 / 25 | Brick size |
| `BRICK_GAP_X/Y` | 8 / 8 | Brick spacing |
| `PADDLE_WIDTH/HEIGHT` | 100 / 20 | Paddle size |
| `PADDLE_SPEED` | 5 | Per-move px (×2 with Alt) |
| `BALL_SIZE` | 20 | Ball square |
| `BALL_SPEED_X/Y` | 3 / 3 | Initial velocity (subtracted) |
| `PADDLE_BOUNDARY_PADDING_X` | 70 | Paddle x limits (inside the walls) |
| `BALL_COLLISION_OFFSET` | 5 | Anti-stick nudge after brick/paddle hit |
| `INTENSE_MUSIC_SCORE_THRESHOLD` | 20 | Score that triggers intense music |
| `GAME_OVER_DELAY_MS` | 2000 | The blocking freeze on round end |
| `KEYPRESS_DELAY_MS` | 200 | Name-entry key repeat throttle |
| `NAME_MAX_LENGTH` | 10 | Name char limit |
| `NUM_GAME_STATES` | 3 | Save slots |
| `NUM_SCORES` | 3 | High-score entries |
| `GAME_STATUS_EMPTY/SAVED/LOST` | 0 / 1 / −1 | `status` magic numbers |

**Dead / misleading constants:** `NUM_BRICKS` (10 — unrelated to the real 45),
`HASH_LENGTH` (10 — unused), and a few layout multipliers in
`render_save_button`. Note them; don't carry the cruft to C++.

---

## Catalogue of quirks, dead code & bugs

The user's word was **"emulation"**, so default to *reproducing* these. Doc 03
turns each into an explicit reproduce-vs-fix decision. This is the
"every little possibility" inventory.

**Behavioural quirks (define the feel — likely keep):**
1. **Win == Lose.** Clearing all bricks ends the round through the same
   loss path (2s freeze, `status = LOST`, game-over). No win screen; a win is
   logged as a loss. *(game.c)*
2. **Alt = double paddle speed**, by applying the move twice. This is the
   README's "move faster." *(game.c:200–217)*
3. **Ball moves by subtraction** of velocity; starts heading up-left.
4. **Anti-stick nudge** after brick (`-= speed-5`) and paddle (`-= speed+5`)
   hits — slightly asymmetric.
5. **Multiple bricks in one frame** each flip `ball_speed_y`; even counts cancel.
6. **Default letter case is uppercase**; Shift gives lowercase (inverted from a
   normal keyboard).

**Bugs (decide deliberately):**
7. **Auto-play clamp is dead code** — paddle can leave the screen in demo mode.
   *(game.c:272–284)*
8. **`SDL_Delay(2000)` blocks the loop** on round end — window freezes 2s.
9. **No fixed timestep** — `update_game` runs once per loop iteration while only
   render is time-gated, so the simulation step-rate is coupled to the
   loop/frame/CPU rate, not wall-clock (physics-tied-to-framerate). *(game.c:380–417)*
10. **Shallow-copy of `GameState`** copies the `char *name` pointer — aliasing/
    leak hazard, only avoided by careful `strdup`/`free`. *(save_system.c, input.c)*
11. **Save format breaks on names with spaces** (no underscore-escaping on the
    save path, unlike the score path).

**Dead code / unused:**
12. `SCREEN_KEYBOARD_QUICK` enum value — never dispatched.
13. `load_records`, `load_dummy_entries`, `swap_entries`, `free_entry_hash`
    in `scores.c` — defined, never called.
14. `temp_collision` field — serialised, never read.
15. `quotes.txt` — never opened (quotes are hard-coded constants).
16. `NUM_BRICKS`, `HASH_LENGTH` constants — unused/misleading.
17. `printf` debug spam throughout `input.c`, `save_system.c`, `ui_screens.c`.

**Hard aborts buried in helpers (anti-pattern to fix in C++):**
18. `get_font_data` → `exit(1)` on null font. *(text.c)*
19. `create_entry` → `exit(EXIT_FAILURE)` on alloc failure. *(scores.c)*

---

## C → C++ idiom map (summary)

The recurring transformations, collected. This table is the cheat-sheet you'll
reach for in every phase.

| C pattern (here) | Idiomatic C++ | Why it's better |
|---|---|---|
| `char *name` + `strdup`/`free` | `std::string` | Automatic memory, value semantics, no aliasing |
| `malloc(sizeof(Button))` + `free` | value type in `std::vector<Button>` | No leaks, no manual count, range-for |
| raw `SDL_Window*`/`Renderer*`/`Texture*`/`Mix_Music*`/`TTF_Font*` | `unique_ptr<T, Deleter>` or RAII class | Destructors free in correct order; `close_app` vanishes |
| `void (*on_click)(Button*)` + `strcmp`-on-label dispatch | `std::function<void()>` per button | Behaviour decoupled from display text |
| `Brick bricks[5][9]` | `std::array<std::array<Brick,9>,5>` | Bounds-aware, iterable, copyable |
| `int status` = -1/0/1 ; `is_playing` = 0/1/2 | `enum class` | Named, type-safe, switch-exhaustive |
| `qsort(…, compare_scores)` | `std::sort(…, lambda)` | Type-safe, inlined, shorter |
| `fscanf`/`fprintf` space-format | `std::ifstream`/`ofstream`, structured format | Safer parsing, no buffer sizing |
| return-`0`-on-failure | exceptions / `std::optional` / `expected` | Errors can't be silently ignored |
| `exit(1)` in a helper | throw / error return | Library code shouldn't kill the app |
| 7× copy-pasted screen loop | a `Screen`/`Menu` class | One place to fix, ~500 fewer lines |
| `AppContext` god-struct passed everywhere | systems as classes owning their data | Encapsulation, testability |
| hand-rolled key-state + 200ms delay | SDL `SDL_TEXTINPUT` events | Correct text entry for free |

---

## Already-resolved issues

For history: the original `Breakout_C/docs/issues.md` listed problems that the
**current** code has already fixed. Don't re-report these — they're done:

- **Recursive state machine → stack overflow** — now an iterative
  `handle_screen_state` loop driven from `main_loop`. ✅
- **Hardcoded Homebrew library paths** — Makefile uses `pkg-config`. ✅
- **OpenSSL SHA-256 dependency** — replaced by in-tree DJB2. ✅
- **Button / text-texture / entry memory leaks** — `destroy_button`, texture
  destroy in `render_text`, and thorough `close_app` cleanup all present. ✅
- **`1000/2000 = 0` frame timing** — now `1000/TARGET_FPS`. ✅
- **Massive `screens.c` god-file** — split into `game.c` / `ui_screens.c` /
  `input.c` / `save_system.c`. ✅
- **Global state** — consolidated into `AppContext` threaded by pointer. ✅
  (Still a god-struct, but no loose globals — the remaining work is the *OO*
  decomposition, which the C++ rewrite does naturally.)

The genuinely-still-open items the rewrite should address are the *quirks/bugs*
above and the *idiom* upgrades — not these.
