#include "player.h"
#include "constants.h"
#include "bn_keypad.h"
#include "bn_sound_items.h"

namespace hb
{

Player::Player() :
    entity{-50, FLOOR_Y - BLOB_HALF_H, 0, 0, true},
    _jump_hold(0),
    _jump_power(0),
    _charge_progress(0)
{
}

void Player::reset_position()
{
    entity.x = -50;
    entity.y = FLOOR_Y - BLOB_HALF_H;
    entity.vx = 0;
    entity.vy = 0;
    entity.on_ground = true;
    _jump_hold = 0;
    _jump_power = 0;
    _charge_progress = 0;
}

void Player::update()
{
    if(bn::keypad::left_held())
    {
        entity.vx = -PLAYER_SPEED;
    }
    else if(bn::keypad::right_held())
    {
        entity.vx = PLAYER_SPEED;
    }
    else
    {
        entity.vx *= bn::fixed(0.8);
    }

    // Charge-on-ground, jump-on-release.
    // While A is held on the ground, build up charge (0..1).
    // When A is released (or charge maxes out), jump with scaled force.
    if(entity.on_ground)
    {
        if(bn::keypad::a_held())
        {
            if(_jump_hold < JUMP_HOLD_MAX) _jump_hold++;
            _charge_progress = bn::fixed(_jump_hold) / bn::fixed(JUMP_HOLD_MAX);
        }
        else if(_jump_hold > 0)
        {
            _jump_power = bn::fixed(_jump_hold) / bn::fixed(JUMP_HOLD_MAX);
            entity.vy = JUMP_FORCE_MIN + (JUMP_FORCE - JUMP_FORCE_MIN) * _jump_power;
            _jump_hold = 0;
            _charge_progress = 0;
            bn::sound_items::sfx_jump.play();
        }
        else
        {
            _charge_progress = 0;
        }
    }
    else
    {
        _jump_hold = 0;
        _charge_progress = 0;
    }

    entity.apply_gravity();

    bn::fixed bottom = FLOOR_Y - BLOB_HALF_H;
    if(entity.y > bottom)
    {
        entity.y = bottom;
        entity.vy = 0;
        entity.on_ground = true;
        _jump_power = 0;
    }

    entity.clamp_to_walls();
    entity.apply_ground_friction();
}

}
