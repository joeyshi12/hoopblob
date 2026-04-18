#include "ball.h"
#include "constants.h"

namespace hb
{

Ball::Ball() :
    entity{0, -20, 0, 0, false},
    _prev_y(-20),
    _prev_x(0)
{
}

BallEvents Ball::update()
{
    BallEvents events;

    _prev_y = entity.y;
    _prev_x = entity.x;
    entity.apply_gravity();

    // Hoop collision (rim edges + backboard). Done before floor/wall so the ball
    // is pushed out cleanly.
    BallEvents he;
    he = _resolve_hoop(LEFT_HOOP_X, true);
    events.rim_hit = events.rim_hit || he.rim_hit;
    he = _resolve_hoop(RIGHT_HOOP_X, false);
    events.rim_hit = events.rim_hit || he.rim_hit;

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
        else
        {
            events.bounced = true;
        }
    }

    // Ceiling bounce
    if(entity.y < CEILING_Y)
    {
        entity.y = CEILING_Y;
        entity.vy = abs_fixed(entity.vy);
        events.bounced = true;
    }

    // Wall bounce
    if(entity.x < WALL_LEFT)
    {
        entity.x = WALL_LEFT;
        entity.vx *= BALL_WALL_BOUNCE;
        events.bounced = true;
    }
    if(entity.x > WALL_RIGHT)
    {
        entity.x = WALL_RIGHT;
        entity.vx *= BALL_WALL_BOUNCE;
        events.bounced = true;
    }

    return events;
}

void Ball::reset(bool to_player_side)
{
    entity.x = to_player_side ? -30 : 30;
    entity.y = -20;
    entity.vx = 0;
    entity.vy = 0;
    _prev_y = entity.y;
    _prev_x = entity.x;
}

void Ball::hit(const Entity& hitter, bn::fixed dir, bn::fixed power)
{
    // power in [0,1]. Below HIT_POWER_MIN the hit is deliberately weak so a
    // tap-jump can't score on its own — holding A charges a stronger shot.
    bn::fixed scale = HIT_POWER_MIN + (bn::fixed(1) - HIT_POWER_MIN) * power;
    entity.vx = HIT_FORCE_X * dir * scale + hitter.vx / 3;
    entity.vy = HIT_FORCE_Y * scale;
}

bool Ball::scored_in_hoop(bn::fixed hoop_x)
{
    // Ball must be within the (narrow) rim opening horizontally
    if(abs_fixed(entity.x - hoop_x) > HOOP_HALF_WIDTH)
    {
        return false;
    }
    // Ball must have crossed downward through the rim Y line this frame
    return _prev_y <= HOOP_RIM_Y && entity.y > HOOP_RIM_Y && entity.vy > 0;
}

BallEvents Ball::_resolve_hoop(bn::fixed hoop_x, bool is_left)
{
    BallEvents e;

    // Backboard: a vertical wall on the outer side of the hoop, above the rim.
    // Its inner face (facing the court) is at hoop_x +/- BACKBOARD_OFFSET.
    bn::fixed board_face_x = is_left ? hoop_x - BACKBOARD_OFFSET
                                     : hoop_x + BACKBOARD_OFFSET;
    if(entity.y >= BACKBOARD_TOP && entity.y <= BACKBOARD_BOTTOM)
    {
        if(is_left)
        {
            // Ball crossed the inner face from the right side (court side) going left
            if(entity.x < board_face_x && _prev_x >= board_face_x && entity.vx < 0)
            {
                entity.x = board_face_x;
                entity.vx = -entity.vx * bn::fixed(0.6);
                e.rim_hit = true;
            }
        }
        else
        {
            if(entity.x > board_face_x && _prev_x <= board_face_x && entity.vx > 0)
            {
                entity.x = board_face_x;
                entity.vx = -entity.vx * bn::fixed(0.6);
                e.rim_hit = true;
            }
        }
    }

    // Rim edges: two small colliders just outside the opening at rim Y.
    // Opening: |x - hoop_x| < HOOP_HALF_WIDTH is the "inside" scoring zone.
    // Rim edge: HOOP_HALF_WIDTH <= |dx| <= HOOP_HALF_WIDTH + RIM_THICKNESS.
    if(abs_fixed(entity.y - HOOP_RIM_Y) <= RIM_Y_BAND)
    {
        bn::fixed dx = entity.x - hoop_x;
        bn::fixed adx = abs_fixed(dx);
        if(adx >= HOOP_HALF_WIDTH && adx <= HOOP_HALF_WIDTH + RIM_THICKNESS)
        {
            // Rim hit: bounce vertically, nudge the ball outward horizontally
            if(entity.vy > 0)
            {
                // Coming down onto the rim: bounce up
                entity.vy = -entity.vy * bn::fixed(0.55);
                entity.y = HOOP_RIM_Y - RIM_Y_BAND;
            }
            else
            {
                // Rising from below into the rim: bounce down
                entity.vy = -entity.vy * bn::fixed(0.55);
                entity.y = HOOP_RIM_Y + RIM_Y_BAND;
            }
            // Push sideways away from the rim center (towards whichever side it was on)
            bn::fixed push_dir = (dx < 0) ? bn::fixed(-1) : bn::fixed(1);
            entity.vx += push_dir * bn::fixed(0.5);
            e.rim_hit = true;
        }
    }

    return e;
}

}
