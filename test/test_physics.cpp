// Test suite for Hoopblob's deterministic physics core (Entity + Ball).
//
// These tests exercise the real game source (src/entity.cpp, src/ball.cpp)
// compiled against a host-side bn::fixed shim. They cover gravity, wall/floor/
// ceiling collisions, friction, rim/backboard deflection, hit force scaling and
// hoop scoring detection — the logic that decides match outcomes.

#include "test_framework.h"
#include "entity.h"
#include "ball.h"
#include "constants.h"

using namespace hb;

namespace
{
    constexpr double EPS = 0.01;
}

// --------------------------------------------------------------------------
// Entity physics
// --------------------------------------------------------------------------

TEST(entity_gravity_accelerates_and_moves)
{
    Entity e{0, 0, 1, 0, true};
    e.apply_gravity();
    // vy gains GRAVITY, position advances by velocity, leaves the ground.
    CHECK_NEAR(e.vy, GRAVITY, EPS);
    CHECK_NEAR(e.x, 1.0, EPS);          // x += vx (1)
    CHECK_NEAR(e.y, GRAVITY, EPS);      // y += vy (now GRAVITY)
    CHECK(e.on_ground == false);
}

TEST(entity_clamps_to_left_wall)
{
    Entity e{-200, 0, -3, 0, false};
    e.clamp_to_walls();
    CHECK_NEAR(e.x, WALL_LEFT, EPS);
    CHECK_NEAR(e.vx, 0.0, EPS);
}

TEST(entity_clamps_to_right_wall)
{
    Entity e{200, 0, 3, 0, false};
    e.clamp_to_walls();
    CHECK_NEAR(e.x, WALL_RIGHT, EPS);
    CHECK_NEAR(e.vx, 0.0, EPS);
}

TEST(entity_does_not_clamp_within_bounds)
{
    Entity e{10, 0, 2, 0, false};
    e.clamp_to_walls();
    CHECK_NEAR(e.x, 10.0, EPS);
    CHECK_NEAR(e.vx, 2.0, EPS);
}

TEST(entity_ground_friction_decays_velocity)
{
    Entity e{0, 0, 10, 0, true};
    e.apply_ground_friction();
    CHECK_NEAR(e.vx, 8.0, EPS);         // 10 * 0.8
}

TEST(entity_no_friction_in_air)
{
    Entity e{0, 0, 10, 0, false};
    e.apply_ground_friction();
    CHECK_NEAR(e.vx, 10.0, EPS);        // unchanged while airborne
}

// --------------------------------------------------------------------------
// Ball physics
// --------------------------------------------------------------------------

TEST(ball_starts_above_center)
{
    Ball ball;
    CHECK_NEAR(ball.entity.x, 0.0, EPS);
    CHECK_NEAR(ball.entity.y, -20.0, EPS);
}

TEST(ball_falls_under_gravity)
{
    Ball ball;
    ball.entity.x = 0;
    ball.entity.y = 0;
    ball.entity.vy = 0;
    BallEvents ev = ball.update();
    CHECK(ball.entity.vy > bn::fixed(0));   // gaining downward velocity
    CHECK(ball.entity.y > bn::fixed(0));    // moved down
    CHECK(ev.bounced == false);
    CHECK(ev.rim_hit == false);
}

TEST(ball_bounces_off_floor)
{
    Ball ball;
    ball.entity.x = 0;
    ball.entity.y = FLOOR_Y - 4;            // at the floor line
    ball.entity.vy = 5;                     // moving down fast
    BallEvents ev = ball.update();
    CHECK(ev.bounced == true);
    CHECK(ball.entity.vy < bn::fixed(0));   // velocity reversed upward
    CHECK(ball.entity.y <= FLOOR_Y - 4 + EPS);
}

TEST(ball_settles_when_slow_on_floor)
{
    Ball ball;
    ball.entity.x = 0;
    ball.entity.y = FLOOR_Y - 4;
    ball.entity.vy = bn::fixed(0.2);        // too slow to keep bouncing
    BallEvents ev = ball.update();
    CHECK_NEAR(ball.entity.vy, 0.0, EPS);   // comes to rest
    CHECK(ev.bounced == false);
}

TEST(ball_bounces_off_right_wall)
{
    Ball ball;
    ball.entity.x = WALL_RIGHT;
    ball.entity.y = 0;
    ball.entity.vx = 4;
    ball.entity.vy = 0;
    BallEvents ev = ball.update();
    CHECK(ev.bounced == true);
    CHECK_NEAR(ball.entity.x, WALL_RIGHT, EPS);
    CHECK(ball.entity.vx < bn::fixed(0));   // reversed away from wall
}

TEST(ball_bounces_off_ceiling)
{
    Ball ball;
    ball.entity.x = 0;
    ball.entity.y = CEILING_Y;
    ball.entity.vy = -4;                    // moving up
    BallEvents ev = ball.update();
    CHECK(ev.bounced == true);
    CHECK_NEAR(ball.entity.y, CEILING_Y, EPS);
    CHECK(ball.entity.vy >= bn::fixed(0));  // pushed back down
}

// --------------------------------------------------------------------------
// Hit force scaling
// --------------------------------------------------------------------------

TEST(ball_full_power_hit_uses_full_force)
{
    Ball ball;
    Entity hitter{0, 0, 0, 0, true};
    ball.hit(hitter, 1, bn::fixed(1));      // dir=right, full charge
    CHECK_NEAR(ball.entity.vx, HIT_FORCE_X, EPS);
    CHECK_NEAR(ball.entity.vy, HIT_FORCE_Y, EPS);
}

TEST(ball_tap_hit_is_weaker_than_full)
{
    Ball ball;
    Entity hitter{0, 0, 0, 0, true};
    ball.hit(hitter, 1, bn::fixed(0));      // dir=right, no charge
    // scale == HIT_POWER_MIN, so horizontal force is HIT_FORCE_X * HIT_POWER_MIN.
    CHECK_NEAR(ball.entity.vx, HIT_FORCE_X * HIT_POWER_MIN, EPS);
    CHECK(ball.entity.vx < HIT_FORCE_X);    // strictly weaker than full
}

TEST(ball_hit_direction_is_respected)
{
    Ball ball;
    Entity hitter{0, 0, 0, 0, true};
    ball.hit(hitter, -1, bn::fixed(1));     // dir=left
    CHECK(ball.entity.vx < bn::fixed(0));   // launched leftward
}

TEST(ball_hit_inherits_hitter_momentum)
{
    Ball still_ball;
    Entity still_hitter{0, 0, 0, 0, true};
    still_ball.hit(still_hitter, 1, bn::fixed(1));

    Ball moving_ball;
    Entity moving_hitter{0, 0, 6, 0, true}; // hitter moving right
    moving_ball.hit(moving_hitter, 1, bn::fixed(1));

    // Moving hitter imparts extra horizontal velocity (vx/3 term).
    CHECK(moving_ball.entity.vx > still_ball.entity.vx);
}

// --------------------------------------------------------------------------
// Scoring detection
// --------------------------------------------------------------------------

TEST(ball_scores_when_dropping_through_rim)
{
    Ball ball;
    ball.entity.x = RIGHT_HOOP_X;           // centered on the rim opening
    ball.entity.y = HOOP_RIM_Y - 1;         // just above the rim line
    ball.entity.vx = 0;
    ball.entity.vy = 2;                     // descending
    ball.update();                          // crosses the rim line this frame
    CHECK(ball.scored_in_hoop(RIGHT_HOOP_X) == true);
}

TEST(ball_does_not_score_outside_opening)
{
    Ball ball;
    ball.entity.x = RIGHT_HOOP_X + 14;      // outside the opening (and rim edge)
    ball.entity.y = HOOP_RIM_Y - 1;
    ball.entity.vx = 0;
    ball.entity.vy = 2;
    ball.update();
    CHECK(ball.scored_in_hoop(RIGHT_HOOP_X) == false);
}

TEST(ball_does_not_score_moving_upward)
{
    Ball ball;
    ball.entity.x = RIGHT_HOOP_X;
    ball.entity.y = HOOP_RIM_Y + 1;         // just below the rim line
    ball.entity.vx = 0;
    ball.entity.vy = -3;                    // rising
    ball.update();
    CHECK(ball.scored_in_hoop(RIGHT_HOOP_X) == false);
}

TEST(ball_deflects_off_rim_edge)
{
    Ball ball;
    // Place the ball right on a rim edge collider, descending.
    ball.entity.x = RIGHT_HOOP_X + bn::fixed(HOOP_HALF_WIDTH) + bn::fixed(1);
    ball.entity.y = HOOP_RIM_Y;
    ball.entity.vx = 0;
    ball.entity.vy = 2;                     // dropping onto the rim
    BallEvents ev = ball.update();
    CHECK(ev.rim_hit == true);
    CHECK(ball.entity.vy < bn::fixed(0));   // bounced back up off the rim
}

// --------------------------------------------------------------------------
// Reset
// --------------------------------------------------------------------------

TEST(ball_reset_to_player_side)
{
    Ball ball;
    ball.entity.vx = 9;
    ball.entity.vy = 9;
    ball.reset(true);
    CHECK_NEAR(ball.entity.x, -30.0, EPS);
    CHECK_NEAR(ball.entity.y, -20.0, EPS);
    CHECK_NEAR(ball.entity.vx, 0.0, EPS);
    CHECK_NEAR(ball.entity.vy, 0.0, EPS);
}

TEST(ball_reset_to_opponent_side)
{
    Ball ball;
    ball.reset(false);
    CHECK_NEAR(ball.entity.x, 30.0, EPS);
}

int main()
{
    return run_all_tests();
}
