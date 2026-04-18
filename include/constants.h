#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "bn_fixed.h"

namespace hb
{
    constexpr bn::fixed FLOOR_Y = 55;
    constexpr bn::fixed CEILING_Y = -70;
    constexpr bn::fixed WALL_LEFT = -115;
    constexpr bn::fixed WALL_RIGHT = 115;
    constexpr bn::fixed GRAVITY = bn::fixed(0.13);
    constexpr bn::fixed PLAYER_SPEED = bn::fixed(1.5);
    constexpr bn::fixed JUMP_FORCE = bn::fixed(-3.4);        // AI / full-charge jump
    constexpr bn::fixed JUMP_FORCE_MIN = bn::fixed(-2.0);    // minimum (short tap release)
    constexpr int JUMP_HOLD_MAX = 25;                        // frames of hold for full charge
    constexpr bn::fixed BALL_BOUNCE = bn::fixed(-0.7);
    constexpr bn::fixed BALL_WALL_BOUNCE = bn::fixed(-0.8);
    constexpr bn::fixed HIT_FORCE_X = bn::fixed(2.5);
    constexpr bn::fixed HIT_FORCE_Y = bn::fixed(-4.5);
    constexpr bn::fixed HIT_POWER_MIN = bn::fixed(0.55); // tap-hit strength fraction
    constexpr int WIN_SCORE = 5;
    constexpr int MATCH_FRAMES = 90 * 60; // 90 seconds at 60 fps

    // Half-height of a blob in world units (sprite is 16 tall, drawn at 1.5x → ~24 tall).
    constexpr bn::fixed BLOB_HALF_H = 12;

    // Hoop geometry. Hoops sit at the left/right edges of the court. The rim is a
    // horizontal opening of half-width HOOP_HALF_WIDTH centered at the hoop X. A
    // ball entering this opening from above and moving downward scores. The rim
    // edges themselves (just outside the opening) deflect the ball. The backboard
    // sits behind the hoop (toward the wall) above the rim, and the ball bounces
    // off its inner face.
    constexpr bn::fixed LEFT_HOOP_X = -100;
    constexpr bn::fixed RIGHT_HOOP_X = 100;
    constexpr bn::fixed HOOP_RIM_Y = -15;
    constexpr bn::fixed HOOP_HALF_WIDTH = 10;   // wider opening (was effectively 8)
    constexpr bn::fixed RIM_THICKNESS = 3;      // how far past the opening the rim edge extends
    constexpr bn::fixed RIM_Y_BAND = 3;         // vertical tolerance for rim edge hit
    // Backboard inner face X (the side that faces the court). For each hoop it is
    // at the hoop X shifted outward (toward its wall). The backboard extends from
    // the rim up to the ceiling.
    constexpr bn::fixed BACKBOARD_OFFSET = 10;  // distance from hoop center to backboard face
    constexpr bn::fixed BACKBOARD_TOP = -40;
    constexpr bn::fixed BACKBOARD_BOTTOM = HOOP_RIM_Y;

    inline bn::fixed abs_fixed(bn::fixed v)
    {
        return v < 0 ? -v : v;
    }
}

#endif
