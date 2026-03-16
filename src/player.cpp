#include "player.h"
#include "constants.h"
#include "bn_keypad.h"

namespace hb
{

Player::Player() :
    entity{-50, FLOOR_Y - 8, 0, 0, true}
{
}

void Player::reset_position()
{
    entity.x = -50;
    entity.y = FLOOR_Y - 8;
    entity.vx = 0;
    entity.vy = 0;
    entity.on_ground = true;
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

    if(bn::keypad::a_pressed() && entity.on_ground)
    {
        entity.vy = JUMP_FORCE;
    }

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
