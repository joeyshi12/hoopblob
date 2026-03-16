#include "opponent.h"
#include "constants.h"

namespace hb
{

Opponent::Opponent() :
    entity{50, FLOOR_Y - 8, 0, 0, true}
{
}

void Opponent::reset_position()
{
    entity.x = 50;
    entity.y = FLOOR_Y - 8;
    entity.vx = 0;
    entity.vy = 0;
    entity.on_ground = true;
    _think_timer = 0;
    _hesitate_timer = 0;
    _wants_jump = false;
    _target_x = 60;
}

void Opponent::update(const Entity& ball, const Entity& player,
                      bn::random& rng, int ai_score, int player_score)
{
    // --- Decision-making (re-evaluate periodically) ---
    _think_timer++;
    if(_think_timer >= 8 + (rng.get_int() % 8))
    {
        _think_timer = 0;

        if(rng.get_int() % 12 == 0)
        {
            _hesitate_timer = 10 + (rng.get_int() % 15);
        }

        // React to player standing on top of us — dodge sideways
        bool player_on_head = player.y < entity.y - 10 &&
                              abs_fixed(player.x - entity.x) < 20 &&
                              !player.on_ground;

        if(player_on_head)
        {
            // Move away from player to shake them off
            _target_x = entity.x + ((player.x > entity.x) ? -25 : 25);
            _wants_jump = false;
        }
        else
        {
            bn::fixed ball_dist_x = ball.x - entity.x;
            bool ball_threatening = ball.x > 20 || ball.vx > bn::fixed(0.5);
            bool ball_on_my_side = ball.x > 0;

            if(ball_threatening || ball_on_my_side)
            {
                _target_x = ball.x + 12;
                _wants_jump = ball.y < entity.y - 10 &&
                              abs_fixed(ball_dist_x) < 30;

                // Occasionally try to jump on player's head for height advantage
                // when near the ball and player is between us and the ball
                if(!_wants_jump && entity.on_ground &&
                   abs_fixed(player.x - entity.x) < 24 &&
                   abs_fixed(ball.x - entity.x) < 40 &&
                   rng.get_int() % 6 == 0)
                {
                    _target_x = player.x;
                    _wants_jump = true;
                }
            }
            else
            {
                _target_x = 60 + (rng.get_int() % 20);
                _wants_jump = false;
            }

            _target_x += bn::fixed(rng.get_int() % 10) - 5;

            if(ai_score < player_score && ball_on_my_side)
            {
                _target_x = ball.x + 8;
                _wants_jump = abs_fixed(ball.x - entity.x) < 35;
            }
        }
    }

    // --- Hesitation ---
    if(_hesitate_timer > 0)
    {
        _hesitate_timer--;
        entity.apply_gravity();
        bn::fixed bottom = FLOOR_Y - 8;
        if(entity.y > bottom)
        {
            entity.y = bottom;
            entity.vy = 0;
            entity.on_ground = true;
        }
        entity.clamp_to_walls();
        return;
    }

    // --- Movement ---
    bn::fixed diff = _target_x - entity.x;
    bn::fixed ai_speed = bn::fixed(1.3);

    if(diff > 2)
    {
        entity.vx = ai_speed;
    }
    else if(diff < -2)
    {
        entity.vx = -ai_speed;
    }
    else
    {
        entity.vx = 0;
    }

    if(_wants_jump && entity.on_ground)
    {
        entity.vy = JUMP_FORCE;
    }

    // --- Physics ---
    entity.apply_gravity();

    bn::fixed bottom = FLOOR_Y - 8;
    if(entity.y > bottom)
    {
        entity.y = bottom;
        entity.vy = 0;
        entity.on_ground = true;
    }

    entity.clamp_to_walls();
    entity.apply_ground_friction();
}

}
