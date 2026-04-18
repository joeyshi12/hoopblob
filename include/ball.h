#ifndef BALL_H
#define BALL_H

#include "entity.h"

namespace hb
{

struct BallEvents
{
    bool bounced = false;   // floor/wall bounce
    bool rim_hit = false;   // rim edge or backboard deflection
};

class Ball
{
public:
    Entity entity;

    Ball();
    BallEvents update();
    void reset(bool to_player_side);
    void hit(const Entity& hitter, bn::fixed dir, bn::fixed power = bn::fixed(1));
    bool scored_in_hoop(bn::fixed hoop_x);

private:
    bn::fixed _prev_y;
    bn::fixed _prev_x;
    BallEvents _resolve_hoop(bn::fixed hoop_x, bool is_left);
};

}

#endif
