# Hoopblob

[![Build and Release](https://github.com/joeyshi12/hoopblob/actions/workflows/build.yml/badge.svg)](https://github.com/joeyshi12/hoopblob/actions/workflows/build.yml)

A GBA basketball game built with the [Butano](https://github.com/GValiente/butano) engine.

Two Kirby-inspired blobs face off on a side-view court, bumping a bouncing ball into each other's hoops. You play on the left against an AI opponent on the right. Matches are time-based, so you need to score more than your opponent before the clock runs out.

## Gameplay

- 90-second match clock with a consolidated scoreboard (your score, time, opponent score) shown on a plaque at the top of the court.
- Sudden-death overtime on ties: first point wins.
- **Charge jump**: hold A on the ground to charge up, release to jump. The longer you hold (up to ~25 frames), the higher the blob jumps and the harder the ball flies off the blob on contact.
- A quick tap of A is intentionally too weak to score, so you have to charge if you want a real shot on the hoop.
- Blobs visibly crouch as they charge a jump, so you can read both your own and the AI's wind-up.

## Controls

| Input | Action |
| --- | --- |
| D-pad left/right | Move (facing is held when no direction is pressed) |
| A | Hold to charge, release to jump |
| Start | Pause (opens a pause menu with Resume / Quit) |
| D-pad up/down | Navigate menus |
| A | Select menu option |
| B | Back (credits screen) |

## Building

### Docker

Requires only [Docker](https://www.docker.com/products/docker-desktop/).

```bash
./build.sh
```

### Native

Requires [devkitPro](https://devkitpro.org/wiki/Getting_Started) with the GBA development tools.

```bash
# Install devkitPro (macOS/Linux)
curl -L https://apt.devkitpro.org/install-devkitpro-pacman | bash
sudo dkp-pacman -S gba-dev

# Set environment
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export PATH=$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH

# Build
make -j$(nproc)
```

## Testing

The deterministic physics core (`Entity` and `Ball`: gravity, collisions, rim
deflection, hit-force scaling and hoop scoring) has a host-side unit test suite
that runs without the GBA toolchain. It compiles the real game source against a
small `bn::fixed` shim, so it works anywhere with just a C++ compiler.

```bash
./test/run_tests.sh

# Any platform, via CMake (e.g. Windows with MSVC)
cmake -S test -B test/build
cmake --build test/build
ctest --test-dir test/build --output-on-failure
```

Tests run automatically in CI on every push and pull request.
