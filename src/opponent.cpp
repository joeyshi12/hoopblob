#include "opponent.h"
#include "constants.h"

namespace hb
{

Opponent::Opponent() :
    entity{50, FLOOR_Y - BLOB_HALF_H, 0, 0, true}
{
}

void Opponent::reset_position()
{
    entity.x = 50;
    entity.y = FLOOR_Y - BLOB_HALF_H;
    entity.vx = 0;
    entity.vy = 0;
    entity.on_ground = true;
    _think_timer = 0;
    _hesitate_timer = 0;
    _wants_jump = false;
    _target_x = 60;
    _charge_frames = 0;
    _charge_target = 0;
    _charge_progress = 0;
}

void Opponent::update(const Entity& ball, const Entity& player,
                      bn::random& rng, int ai_score, int player_score)
{
    // --- Decision-making (re-evaluate periodically) ---
    _think_timer++;
    bn::fixed ball_abs_vx = ball.vx < 0 ? -ball.vx : ball.vx;
    bool ball_threatening = ball_abs_vx > bn::fixed(0.5) || ball.x > -40;

    if(_think_timer >= 8 + (rng.get_int() % 8))
    {
        _think_timer = 0;

        // Only hesitate when the ball isn't a threat — otherwise AI looks frozen
        // right when the player is attacking.
        if(!ball_threatening && rng.get_int() % 12 == 0)
        {
            _hesitate_timer = 10 + (rng.get_int() % 15);
        }

        // React to player standing on top of us — dodge sideways
        bool player_on_head = player.y < entity.y - 10 &&
                              abs_fixed(player.x - entity.x) < 20 &&
                              !player.on_ground;

        if(player_on_head)
        {
            _target_x = entity.x + ((player.x > entity.x) ? -25 : 25);
            _wants_jump = false;
        }
        else
        {
            // Stand on the side of the ball that lets us hit it toward the
            // player's hoop (left). So target = ball.x + offset (positive).
            // When the ball is behind us (to our right), we have to come back
            // around — target the ball directly until we pass it.
            bn::fixed offset = (ball.x < entity.x) ? bn::fixed(10) : bn::fixed(-6);
            _target_x = ball.x + offset;

            // Jump when the ball is above us and within reach horizontally.
            _wants_jump = ball.y < entity.y - 6 &&
                          abs_fixed(ball.x - entity.x) < 28;

            _target_x += bn::fixed(rng.get_int() % 8) - 4;

            // If losing, play more aggressively toward the ball.
            if(ai_score < player_score)
            {
                _target_x = ball.x + ((ball.x < entity.x) ? bn::fixed(6) : bn::fixed(-4));
                if(abs_fixed(ball.x - entity.x) < 40)
                {
                    _wants_jump = _wants_jump || ball.y < entity.y;
                }
            }
        }
    }

    // --- Hesitation ---
    if(_hesitate_timer > 0)
    {
        _hesitate_timer--;
        entity.apply_gravity();
        bn::fixed bottom = FLOOR_Y - BLOB_HALF_H;
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

    // Charge-and-jump: enter a brief crouch before launching so the player can
    // see the AI winding up (mirrors the player's charge-jump mechanic).
    if(entity.on_ground)
    {
        if(_charge_target > 0)
        {
            // Already charging — keep winding up, don't move.
            entity.vx = 0;
            _charge_frames++;
            _charge_progress = bn::fixed(_charge_frames) / bn::fixed(_charge_target);
            if(_charge_frames >= _charge_target)
            {
                entity.vy = JUMP_FORCE;
                _charge_target = 0;
                _charge_frames = 0;
                _charge_progress = 0;
            }
        }
        else if(_wants_jump)
        {
            // Start a new charge. Randomize duration slightly for variety.
            _charge_target = 10 + (rng.get_int() % 8); // ~10-17 frames
            _charge_frames = 0;
            _charge_progress = 0;
            _wants_jump = false;
            entity.vx = 0;
        }
    }
    else
    {
        _charge_target = 0;
        _charge_frames = 0;
        _charge_progress = 0;
    }

    // --- Physics ---
    entity.apply_gravity();

    bn::fixed bottom = FLOOR_Y - BLOB_HALF_H;
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
