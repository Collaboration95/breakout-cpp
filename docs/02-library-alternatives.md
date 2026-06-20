# 02 — Library Alternatives

You asked to weigh alternatives to SDL2 "along the way." This doc lays out the
landscape so you can decide *with* the rewrite rather than *before* it.

**The recommendation up front, then the reasoning:**

> **Do the C++ port on SDL2 first.** Then, if you want, treat a library
> migration as an optional later track (Phase 11+ in Doc 03).

## Why not switch libraries at the same time as the language

You're changing two huge things at once already — **C → C++** (new language,
new idioms, new mental model) and **procedural → object-oriented** (new
architecture). Adding **SDL2 → something else** as a *third* simultaneous change
is how rewrites stall: when something breaks, you can't tell whether it's a C++
mistake, a design mistake, or a library you don't know yet.

SDL2 is also the part you *already know*. Keeping it means every bug during the
port is unambiguously a **C++/design** bug — which is exactly the skill you're
here to build. The existing code, the resources (PNG/TTF/WAV), and the data
files all already work with SDL2; reusing them keeps each phase short and
playable.

Once the C++ version is complete and faithful, swapping the rendering/audio
backend becomes a *clean, isolated* exercise — and a genuinely good one, because
a well-designed C++ port will have hidden SDL behind your own `Renderer` /
`AudioPlayer` / `TextRenderer` classes. Migration then means rewriting *those few
classes*, not the whole game. (That payoff is itself a reason to do the OO
abstraction well — see Doc 03 Phase 0.)

---

## The whole-stack options

What replaces "SDL2 + SDL2_image + SDL2_ttf + SDL2_mixer" as a bundle.

### SDL2 (the incumbent) — *recommended for the port*
- **What it is:** mature C library for window/input/2D-render/audio; the satellite
  libs add PNG (`_image`), TrueType (`_ttf`), and mixing (`_mixer`).
- **Pros:** you already know it; the current game maps 1:1; enormous docs and
  Stack Overflow corpus; rock stable; everything already builds via `pkg-config`.
- **Cons:** C API (manual create/destroy — but that's *the RAII lesson*);
  four separate libraries to install; immediate-mode only.
- **C++ angle:** the C API is *why* it's a good first C++ project — you get to
  build the RAII wrappers yourself and feel the payoff.

### SDL3 — *the natural "modernise the same thing" step*
- **What it is:** the next major SDL. Same model, cleaned-up API; **image, audio
  mixing, and TTF are being folded in / restructured** (`SDL3_image`,
  `SDL3_mixer`, `SDL3_ttf` exist as the satellite set), and there's a new GPU
  API.
- **Pros:** closest possible migration from SDL2 (concepts transfer almost
  directly); actively developed; better callbacks/main-loop story.
- **Cons:** newer ⇒ fewer tutorials, some churn; function signatures differ
  (e.g. many `bool`-returning calls, `SDL_Render*` renames, `SDL_FRect`
  float rects); your muscle memory is SDL2.
- **C++ angle:** SDL3 uses **float rects** natively — which nudges you toward
  float-based physics (a fix for the integer-locked movement). A good *second*
  target precisely because it's a small delta.

### SFML — *the "C++-native" option*
- **What it is:** Simple and Fast Multimedia Library — an actual **C++** library:
  classes, RAII, operator overloads, `sf::RenderWindow`, `sf::Sprite`,
  `sf::Text`, `sf::Music`, `sf::Sound` all built in (no separate image/ttf/mixer
  installs).
- **Pros:** idiomatic C++ out of the box; one dependency covers graphics + audio
  + fonts + window; very beginner-friendly; SFML 3 (2024) modernised the API
  (scoped enums, `std::optional` events).
- **Cons:** you'd be *learning C++ idioms from SFML* rather than *building them
  yourself* — less of the manual-RAII lesson; smaller ecosystem than SDL; ties
  you to SFML's object model.
- **C++ angle:** the most "everything is already a C++ object" experience. Great
  if your priority is *shipping idiomatic C++ fast*; less ideal if your priority
  is *understanding why* RAII/ownership matter (since SDL's C API teaches that by
  contrast).

### raylib — *the "minimum friction" option*
- **What it is:** a small, fun C game library with batteries included (textures,
  fonts, audio, shapes) and an extremely simple immediate-mode API
  (`InitWindow`, `BeginDrawing`, `DrawRectangle`, `DrawText`, `PlayMusicStream`).
- **Pros:** the least boilerplate of anything here; one dependency; delightful
  for prototyping; has a C++ binding (`raylib-cpp`) that adds RAII wrappers.
- **Cons:** C API like SDL (so same manual-resource story unless you use
  `raylib-cpp`); opinionated defaults; less control over the render pipeline;
  smaller "serious app" ecosystem.
- **C++ angle:** with `raylib-cpp` you get RAII for free; without it, it's
  SDL-like. Best if you want to *see the game working fast* and care less about
  matching the original's exact SDL feel.

### Quick comparison

| | SDL2 | SDL3 | SFML 3 | raylib |
|---|---|---|---|---|
| Native language | C | C | **C++** | C (C++ via raylib-cpp) |
| One-dep media stack | ✗ (4 libs) | partial | **✓** | **✓** |
| You already know it | **✓✓** | ~ | ✗ | ✗ |
| Teaches manual RAII | **✓✓** | ✓ | ✗ (gives it to you) | ✗ / ✓ |
| Float-based geometry | ✗ (int rects) | **✓** | ✓ | ✓ |
| Tutorial mass | **✓✓** | ✓ | ✓ | ✓ |
| Effort to port *this* game | **lowest** | low | medium | medium |
| Good *migration* target later | — | **✓✓** | ✓ | ✓ |

---

## Per-subsystem swaps (mix-and-match)

If you stay on SDL2 core, you can still modernise individual pieces. Useful
because it isolates one new thing at a time.

### Image loading — replacing `SDL2_image`
- **`stb_image.h`** (single-header, public-domain): drops the `SDL2_image`
  dependency; you `stbi_load` a PNG into pixels and hand them to
  `SDL_CreateTexture` / `SDL_UpdateTexture`. Great "I understand what a texture
  *is*" lesson and one fewer install. **Recommended swap** if you want to trim
  deps.
- Keep `SDL2_image` if you'd rather not think about pixel formats yet.

### Font / text — replacing `SDL2_ttf`
- **`stb_truetype.h`** (single-header): rasterise glyphs yourself — educational
  but a real chunk of work (atlas building, kerning). Probably *not* worth it for
  a first C++ project.
- **Keep `SDL2_ttf`**, but **add a glyph/text-texture cache** in your
  `TextRenderer` (the C re-renders every string every frame). This is the
  high-value change, not replacing the lib.

### Audio — replacing `SDL2_mixer`
- **`miniaudio.h`** (single-header): a full audio engine in one file —
  playback, decoding (WAV/MP3/FLAC), mixing. Cleanly wraps in an `AudioPlayer`
  class and removes `SDL2_mixer`. **Recommended swap** if you want a modern,
  self-contained audio layer.
- **Keep `SDL2_mixer`** for the most direct port (the four-track logic maps
  straight over).

### Score integrity hash — replacing DJB2
- The DJB2 hash is already dependency-free and fine. If you ever want a "proper"
  one without a heavy dep, a **single-header SHA-256** (e.g. a public-domain
  `sha256.h`) or `std::hash` composition works. **Lowest priority** — leave it
  unless you specifically want a crypto/hashing lesson.

### Serialization (save/score files)
- The fragile space-separated format is the most tempting thing to replace.
  Options: keep it (most faithful), one-field-per-line (trivial, robust), or a
  header-only **JSON** lib (`nlohmann/json`) for a real "serialize a struct"
  C++ lesson. Pick per how much you want to fight the format vs. learn from it.

---

## Build system: Make vs CMake

Tangential to the libraries but you'll decide it in Phase 0.

- **Keep `make`:** simplest, you already have a working Makefile to adapt
  (switch `clang`→`clang++`/`g++`, `-std=c99`→`-std=c++20`). Fine for one binary.
- **Switch to CMake:** the de-facto C++ standard; far easier to pull in
  header-only libs, find SDL across platforms (`find_package`), wire up a test
  target, and (later) swap backends. **Recommended** — learning CMake is part of
  learning the C++ ecosystem, and it makes the optional library migration much
  less painful. `FetchContent` can vendor SFML/raylib/SDL3 with a few lines when
  you get there.

---

## Bottom line

1. **Port on SDL2 + CMake + C++20.** Hide SDL behind your own thin classes.
2. **Optionally swap single-header libs** for individual subsystems once the
   game runs (`stb_image`, `miniaudio`) — each is one isolated lesson.
3. **Save a full backend migration (SDL3 / SFML / raylib) for last**, when it's
   a contained rewrite of a handful of wrapper classes — and a satisfying proof
   that your abstractions were good.

Doc 03 builds the roadmap around exactly this: SDL2 throughout, with the
migration parked as an explicit, optional final phase.
