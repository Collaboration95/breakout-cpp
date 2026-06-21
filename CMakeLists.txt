cmake_minimum_required(VERSION 3.25)
project(Breakout LANGUAGES CXX)

# ── Language standard ────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Tooling support ──────────────────────────────────────────────
# Writes build/compile_commands.json — clangd (IntelliSense) reads it.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Sources ──────────────────────────────────────────────────────
# Picks up every .cpp under src/ automatically, including subdirectories.
# CONFIGURE_DEPENDS re-checks the glob on every build so new files are
# included without editing this file.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(breakout ${SOURCES})

# Every file in src/ can #include each other without path prefixes.
target_include_directories(breakout PRIVATE src)

# ── Compile flags ────────────────────────────────────────────────
target_compile_options(breakout PRIVATE
    -O0                 # no optimisation — keep debug info readable
    -g                  # full debug symbols
    -Wall               # standard warnings
    -Wextra             # extra warnings beyond -Wall
    -Weffc++            # Scott Meyers "Effective C++" checks (GCC only)
    -Wconversion        # warn on implicit type-narrowing conversions
    -Wsign-conversion   # warn when signed/unsigned mismatch silently converts
)

# ── SDL2 ─────────────────────────────────────────────────────────
find_package(SDL2 REQUIRED)
target_link_libraries(breakout PRIVATE SDL2::SDL2 SDL2::SDL2main)
