#ifndef BALL_H
#define BALL_H

#include "entity.h"

namespace hb
{

class Ball
{
public:
    Entity entity;

    Ball();
    void update();
    void reset(bool to_player_side);
    void hit(const Entity& hitter, bn::fixed dir);
    bool scored_in_hoop(bn::fixed hoop_x);

private:
    bn::fixed _prev_y;
};

}

#endif
