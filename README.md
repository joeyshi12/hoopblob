# Hoopblob

A GBA basketball game built with the [Butano](https://github.com/GValiente/butano) engine.

Two Kirby-inspired blobs face off on a side-view court, bumping a bouncing ball into each other's hoops. You play on the left against an AI opponent on the right. Matches are time-based — score more than your opponent before the clock runs out.

## Gameplay

- 90-second match clock with a consolidated scoreboard (your score, time, opponent score) shown on a plaque at the top of the court.
- Sudden-death overtime on ties — first point wins.
- **Charge jump**: hold A on the ground to charge up, release to jump. The longer you hold (up to ~25 frames), the higher the blob jumps and the harder the ball flies off the blob on contact.
- A quick tap of A is intentionally too weak to score — you have to charge if you want a real shot on the hoop.
- Blobs visibly crouch as they charge a jump, so you can read both your own and the AI's wind-up.

## Controls

- D-pad left/right — Move (facing is held when no direction is pressed)
- A — Hold to charge, release to jump
- Start — Pause (opens a pause menu with Resume / Quit)
- D-pad up/down — Navigate menus
- A — Select menu option
- B — Back (credits screen)

## Building

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
