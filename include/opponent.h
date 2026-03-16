#ifndef OPPONENT_H
#define OPPONENT_H

#include "entity.h"
#include "bn_random.h"

namespace hb
{

class Opponent
{
public:
    Entity entity;

    Opponent();
    void update(const Entity& ball_entity, const Entity& player_entity,
                bn::random& rng, int ai_score, int player_score);
    void reset_position();

private:
    int _think_timer = 0;
    bn::fixed _target_x = 60;
    bool _wants_jump = false;
    int _hesitate_timer = 0;
};

}

#endif
