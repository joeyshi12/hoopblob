#ifndef ENTITY_H
#define ENTITY_H

#include "bn_fixed.h"

namespace hb
{

// Lightweight POD-like struct — no vtable, used in hot loop every frame.
struct Entity
{
    bn::fixed x;
    bn::fixed y;
    bn::fixed vx;
    bn::fixed vy;
    bool on_ground = false;

    void apply_gravity();
    void clamp_to_walls();
    void apply_ground_friction();
};

}

#endif
