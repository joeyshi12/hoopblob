#include "entity.h"
#include "constants.h"

namespace hb
{

void Entity::apply_gravity()
{
    vy += GRAVITY;
    x += vx;
    y += vy;
    on_ground = false;
}

void Entity::clamp_to_walls()
{
    if(x < WALL_LEFT)
    {
        x = WALL_LEFT;
        vx = 0;
    }
    if(x > WALL_RIGHT)
    {
        x = WALL_RIGHT;
        vx = 0;
    }
}

void Entity::apply_ground_friction()
{
    if(on_ground)
    {
        vx *= bn::fixed(0.8);
    }
}

}
