#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"

namespace hb
{

class Player
{
public:
    Entity entity;

    Player();
    void update();
    void reset_position();
};

}

#endif
