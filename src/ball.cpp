#include "ball.h"
#include "constants.h"

namespace hb
{

Ball::Ball() :
    entity{0, -20, 0, 0, false},
    _prev_y(-20)
{
}

void Ball::update()
{
    _prev_y = entity.y;
    entity.apply_gravity();

    // Floor bounce
    bn::fixed bottom = FLOOR_Y - 4;
    if(entity.y > bottom)
    {
        entity.y = bottom;
        entity.vy *= BALL_BOUNCE;
        entity.vx *= bn::fixed(0.95);
        if(abs_fixed(entity.vy) < bn::fixed(0.5))
        {
            entity.vy = 0;
        }
    }

    // Ceiling bounce
    if(entity.y < CEILING_Y)
    {
        entity.y = CEILING_Y;
        entity.vy = abs_fixed(entity.vy);
    }

    // Wall bounce
    if(entity.x < WALL_LEFT)
    {
        entity.x = WALL_LEFT;
        entity.vx *= BALL_WALL_BOUNCE;
    }
    if(entity.x > WALL_RIGHT)
    {
        entity.x = WALL_RIGHT;
        entity.vx *= BALL_WALL_BOUNCE;
    }
}

void Ball::reset(bool to_player_side)
{
    entity.x = to_player_side ? -30 : 30;
    entity.y = -20;
    entity.vx = 0;
    entity.vy = 0;
    _prev_y = entity.y;
}

void Ball::hit(const Entity& hitter, bn::fixed dir)
{
    entity.vx = HIT_FORCE_X * dir + hitter.vx / 3;
    entity.vy = HIT_FORCE_Y;
}

bool Ball::scored_in_hoop(bn::fixed hoop_x)
{
    // Ball must be within the rim opening horizontally
    if(abs_fixed(entity.x - hoop_x) > 8)
    {
        return false;
    }

    // Ball must have crossed downward through the rim Y line this frame
    // (was above or at rim, now below it)
    return _prev_y <= HOOP_RIM_Y && entity.y > HOOP_RIM_Y && entity.vy > 0;
}

}
