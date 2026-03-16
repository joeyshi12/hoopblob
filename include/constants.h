#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "bn_fixed.h"

namespace hb
{
    constexpr bn::fixed FLOOR_Y = 55;
    constexpr bn::fixed CEILING_Y = -70;
    constexpr bn::fixed WALL_LEFT = -115;
    constexpr bn::fixed WALL_RIGHT = 115;
    constexpr bn::fixed GRAVITY = bn::fixed(0.15);
    constexpr bn::fixed PLAYER_SPEED = bn::fixed(1.5);
    constexpr bn::fixed JUMP_FORCE = bn::fixed(-3.5);
    constexpr bn::fixed BALL_BOUNCE = bn::fixed(-0.7);
    constexpr bn::fixed BALL_WALL_BOUNCE = bn::fixed(-0.8);
    constexpr bn::fixed HIT_FORCE_X = bn::fixed(2.5);
    constexpr bn::fixed HIT_FORCE_Y = bn::fixed(-3.0);
    constexpr int WIN_SCORE = 10;
    constexpr bn::fixed LEFT_HOOP_X = -100;
    constexpr bn::fixed RIGHT_HOOP_X = 100;
    constexpr bn::fixed HOOP_RIM_Y = -15;

    inline bn::fixed abs_fixed(bn::fixed v)
    {
        return v < 0 ? -v : v;
    }
}

#endif
