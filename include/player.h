#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "bn_fixed.h"

namespace hb
{

class Player
{
public:
    Entity entity;

    Player();
    void update();
    void reset_position();

    // Jump power captured on the jump that produced the current air-time.
    // 0 = tap, 1 = fully-charged hold. Used to scale how hard a hit strikes.
    [[nodiscard]] bn::fixed jump_power() const { return _jump_power; }

    // Current on-ground charge progress (0..1) while holding A to charge a jump.
    [[nodiscard]] bn::fixed charge_progress() const { return _charge_progress; }

private:
    int _jump_hold;        // frames A has been held while on the ground
    bn::fixed _jump_power; // 0..1 snapshot of last jump's charge
    bn::fixed _charge_progress; // 0..1 live charge while holding on the ground
};

}

#endif
