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

    // 0..1 charge progress while winding up to jump (mirrors Player).
    [[nodiscard]] bn::fixed charge_progress() const { return _charge_progress; }

private:
    int _think_timer = 0;
    bn::fixed _target_x = 60;
    bool _wants_jump = false;
    int _hesitate_timer = 0;
    int _charge_frames = 0;        // frames spent in crouch before jumping
    int _charge_target = 0;        // total crouch duration this wind-up (0 = not charging)
    bn::fixed _charge_progress = 0;
};

}

#endif
