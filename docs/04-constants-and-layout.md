# 04 — Constants, Geometry & Layout (the "never open the source" reference)

Everything numeric, in one place, so you can rebuild pixel-accurately without
reading `constants.h` or recomputing the derived positions. Doc 01 explains
*behaviour*; this doc is the *numbers*.

All values are from the C `constants.h` plus the small formulas in `game.c` /
`main.c`, with the **derived results pre-computed** for the default 864×558
window. (Verified: the computed paddle/ball start positions match the shipped
`savefile.txt` exactly — `382 463 100 20` / `422 269 20 20`.)

---

## Window & SDL init parameters

| Thing | Value |
|---|---|
| Window size | **864 × 558** |
| Window flags | `SDL_WINDOW_SHOWN` |
| Renderer flags | `SDL_RENDERER_ACCELERATED \| SDL_RENDERER_PRESENTVSYNC` |
| Blend mode | `SDL_BLENDMODE_BLEND` (needed for the alpha brick shades) |
| Image init | `IMG_INIT_PNG` |
| Audio (`Mix_OpenAudio`) | freq **44100**, `MIX_DEFAULT_FORMAT`, **2** channels, **2048** chunk |
| TTF | `TTF_Init()`; font opened at size 60, then resized per draw |
| Initial window title | `"Background Display"` (overwritten per screen) |

Per-screen window titles: `Main Menu`, `Instructions Panel` → `Game in Progress`,
`Load Game`, `Quit Game ?`, `Game Over :(`, `High Scores`, `Settings`,
`Save Game`, `Enter Name`.

---

## Gameplay tunables

| Constant | Value | Meaning |
|---|---|---|
| `TARGET_FPS` | 60 | Render cap → `1000/60 = 16` ms/frame |
| `PADDLE_WIDTH` × `PADDLE_HEIGHT` | — | Paddle exists (any reasonable size) |
| `PADDLE_SPEED` | — | Paddle moves left/right; faster while Left-Alt held (any faster amount) |
| `PADDLE_BOUNDARY_PADDING_X` | — | Paddle kept inside horizontal bounds (any padding that keeps it visible) |
| `BALL_SIZE` | 20 | Ball is 20×20 |
| `BALL_SPEED_X` / `BALL_SPEED_Y` | 3 / 3 | Initial velocity (**subtracted** from pos → starts up-left) |
| `BALL_COLLISION_OFFSET` | 5 | Anti-stick nudge after brick/paddle hit |
| `INTENSE_MUSIC_SCORE_THRESHOLD` | 20 | `score > 20` → switch to intense track |
| `GAME_OVER_DELAY_MS` | 2000 | Blocking freeze on round end (Doc 03 Phase 4: fix this) |
| `KEYPRESS_DELAY_MS` | 200 | Name-entry key-repeat throttle |
| `NAME_MAX_LENGTH` | 10 | Max name characters |
| `NAME_BUFFER_SIZE` | 512 | `player_name` buffer size |
| `NUM_GAME_STATES` | 3 | Save slots |
| `NUM_SCORES` | 3 | High-score rows |
| `PADDLE_START_Y_RATIO` | — | Paddle exists near bottom (any visible y) |

**`status` codes:** `GAME_STATUS_EMPTY = 0`, `GAME_STATUS_SAVED = 1`,
`GAME_STATUS_LOST = -1`.

**Misleading/dead constants (don't port):** `NUM_BRICKS = 10` (unrelated to the
real 45), `HASH_LENGTH = 10` (unused), `HASH_SIZE = 256` (parse buffer only).

---

## Brick grid

| Constant | Value |
|---|---|
| `NUM_BRICK_ROWS` × `NUM_BRICK_COLS` | 5 × 9 (= **45 bricks**) |
| `BRICK_WIDTH` × `BRICK_HEIGHT` | 70 × 25 |
| `BRICK_GAP_X` / `BRICK_GAP_Y` | 8 / 8 |
| `BRICK_START_Y` | 60 |

**Derived layout** (centered horizontally):
```
brick_start_x = (864 − (9·70 + 8·8)) / 2 = (864 − 694) / 2 = 85
brick[row][col].x = 85 + col·(70+8) = 85 + 78·col      (col 0..8 → 85,163,…,709)
brick[row][col].y = 60 + row·(25+8) = 60 + 33·row      (row 0..4 → 60,93,126,159,192)
```
**Initial health:** `health = NUM_BRICK_ROWS − row = 5 − row`
→ row 0 (top) = 5 … row 4 (bottom) = 1. **Higher health = drawn darker.**

---

## Walls (exact rects)

Built in `init_game`; these are the collision surfaces.

| Wall | Source formula | **Computed rect (x, y, w, h)** | Bounce |
|---|---|---|---|
| Left | `(66, 55, 5, 420)` | **(66, 55, 5, 420)** | flip `ball_speed_x` |
| Right | `(864−71, 55, 5, 420)` | **(793, 55, 5, 420)** | flip `ball_speed_x` |
| Top | `(83, 45, 700, 5)` | **(83, 45, 700, 5)** | flip `ball_speed_y` |
| Bottom | `(83, 480, 700, 5)` | **(83, 480, 700, 5)** | round over (lose) |

---

## Paddle & ball start positions (derived)

| Entity | Formula | **Computed (x, y, w, h)** |
|---|---|---|
| Paddle | — | **Paddle exists, visible, near bottom-center (any reasonable rect)** |
| Ball | `(864/2 − 20/2, 558/2 − 20/2, 20, 20)` | **(422, 269, 20, 20)** |
| Paddle x range | — | **Paddle stays within window bounds (any clamping that keeps it visible)** |

---

## Colours (exact RGBA)

Drawing uses `SDL_SetRenderDrawColor`; text is always solid black.

| Use | RGBA |
|---|---|
| Paddle, walls, ball, text | `0, 0, 0, 255` (opaque black) |
| Brick health 5 | `0, 0, 0, 255` (100%) |
| Brick health 4 | `0, 0, 0, 204` (80%) |
| Brick health 3 | `0, 0, 0, 153` (60%) |
| Brick health 2 | `0, 0, 0, 102` (40%) |
| Brick health 1 | `0, 0, 0, 51` (20%) |
| Brick health 0 | not drawn (destroyed) |
| Menu clear colour | `0, 0, 0, 0` |

(The alpha blend over the white background panel is what makes tougher bricks
look darker. This is why `SDL_BLENDMODE_BLEND` is set at init.)

---

## Fonts

One shared `TTF_Font` (`LondrinaSolid-Light.ttf`), resized per call via
`TTF_SetFontSize`.

| Name | Size | Used for |
|---|---|---|
| `TITLE_FONT_SIZE` | 60 | Screen titles ("Breakout", "Game Over", …) |
| `MEDIUM_FONT_SIZE` | 40 | Slot button labels, name-entry name |
| `QUOTE_FONT_SIZE` | 30 | Taglines, score HUD, instructions |
| `DEFAULT_TEXT_SIZE` | 25 | Slot status, settings label |
| `BUTTON_FONT_SIZE` | 22 | Button labels |
| `SMALL_FONT_SIZE` | 20 | Slot name/score, input hints |

---

## Vertical layout ratios

Text Y positions are `ratio × SCREEN_HEIGHT` (558). Pre-computed px in the last
column.

| Constant | Ratio | ≈ px | Used for |
|---|---|---|---|
| `TITLE_Y_RATIO` | 0.16 | 89 | Screen title line |
| `QUOTE_Y_RATIO` | 0.25 | 139 | Menu taglines / "Game Saved" |
| `INSTRUCTION_TEXT_Y_RATIO` | 0.36 | 200 | "Use ← →" instruction |
| `GAME_OVER_SCORE_Y_RATIO` | 0.45 | 251 | Final score on game-over |
| `SCORE_DISPLAY_Y_RATIO` | 0.55 | 306 | In-game `[Score: N]` HUD |
| `HIGH_SCORE_START_Y_RATIO` | 0.20 | 111 | First high-score row (+`40`/row) |
| `SLOT_BUTTON_Y_RATIO` | 0.23 | 128 | First save slot (+`110`/slot) |
| `INPUT_NAME_Y_RATIO` | 0.20 | 111 | Typed name |
| `INPUT_UNDERLINE_Y_RATIO` | 0.24 | 134 | Underline under the name |
| `INPUT_LIMIT_Y_RATIO` | 0.75 | 419 | "[10 character input limit]" |
| `INPUT_HELP_Y_RATIO` | 0.80 | 446 | "[Enter Name from Keyboard]" |
| `SETTINGS_LABEL_X_RATIO` / `_Y_RATIO` | 0.70 / 0.35 | — / 195 | "Automatic Mode :" label |

`HIGH_SCORE_LINE_SPACING = 40`, `SLOT_BUTTON_SPACING = 110`.

---

## Button positions (exact, per screen)

All buttons are `SDL_Rect` (x, y, w, h). The label text is also the click-id in
the C (don't reproduce that — Doc 03 Phase 6).

**Back button (shared):** `(20, 20, 80, 40)`.

**Main menu** — `120 × 50`, x-left 295 / x-right 425, y-top 240 / y-bottom 300:
| Label | Rect |
|---|---|
| Quick Game | (295, 240, 120, 50) |
| Load Game | (425, 240, 120, 50) |
| High Score | (295, 300, 120, 50) |
| Settings | (425, 300, 120, 50) |

**Save slots (Load & Save screens)** — `600 × 100`, centered (x = 432 − 300 = 132):
| Slot | Rect |
|---|---|
| Slot 1 | (132, 128, 600, 100) |
| Slot 2 | (132, 238, 600, 100) |
| Slot 3 | (132, 348, 600, 100) |

**Game-over** — `120 × 50`, y 280:
| Label | Rect |
|---|---|
| Main Menu | (295, 280, 120, 50) |
| High Score | (435, 280, 120, 50) |

**Quit menu** — `120 × 50`:
| Label | Rect | Shown when |
|---|---|---|
| Yes | (300, 220, 120, 50) | always |
| Go Back | (450, 220, 120, 50) | always |
| Save Game | (375, 280, 120, 50) | came from GAME with `selected_slot == 0` |

**Settings:**
| Label | Rect |
|---|---|
| Enable / Disable (toggle) | (425, 175, 120, 50) |
| Reset Save File | (360, 250, 150, 50) |

**Name-entry (keyboard):**
| Label | Rect |
|---|---|
| Save Name | (375, 240, 120, 50) |

---

## Controls (complete)

| Input | Effect |
|---|---|
| ← / → | Move paddle |
| Left Alt + ← / → | Move paddle faster (any amount faster) |
| Space | Start the round (from the instruction panel) |
| Esc | Open the quit menu (saves first, in-game) |
| A–Z | Type a name (uppercase by default; **Shift = lowercase**) |
| Backspace | Delete last name char |
| Enter / Return | Confirm name |
| Mouse click | Press buttons |
| Window ✕ | Treated like Esc (→ quit menu) |
