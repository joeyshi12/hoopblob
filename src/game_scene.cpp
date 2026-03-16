#include "game_scene.h"
#include "constants.h"
#include "player.h"
#include "opponent.h"
#include "ball.h"
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_string.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_items_player.h"
#include "bn_sprite_items_opponent.h"
#include "bn_sprite_items_ball.h"
#include "bn_sprite_items_hoop.h"

namespace hb
{

namespace
{
    constexpr int COUNTDOWN_FRAMES = 60; // ~1 second per number, 3 numbers

    bool check_collision(const Entity& a, bn::fixed aw, bn::fixed ah,
                         const Entity& b, bn::fixed bw, bn::fixed bh)
    {
        return abs_fixed(a.x - b.x) < (aw + bw) / 2 &&
               abs_fixed(a.y - b.y) < (ah + bh) / 2;
    }

    void update_score_display(bn::sprite_text_generator& text_gen,
                              bn::vector<bn::sprite_ptr, 8>& sprites,
                              int player_score, int opponent_score)
    {
        sprites.clear();

        text_gen.set_left_alignment();
        bn::string<8> ps;
        bn::ostringstream ps_stream(ps);
        ps_stream << player_score;
        text_gen.generate(-115, -72, ps, sprites);

        text_gen.set_right_alignment();
        bn::string<8> os;
        bn::ostringstream os_stream(os);
        os_stream << opponent_score;
        text_gen.generate(115, -72, os, sprites);

        text_gen.set_center_alignment();
    }

    // Runs a 3-2-1 countdown, updating sprites each frame
    void run_countdown(bn::sprite_text_generator& text_gen)
    {
        bn::vector<bn::sprite_ptr, 4> cd_sprites;
        text_gen.set_center_alignment();

        for(int i = 3; i >= 1; i--)
        {
            cd_sprites.clear();
            bn::string<4> num;
            bn::ostringstream num_stream(num);
            num_stream << i;
            text_gen.generate(0, 0, num, cd_sprites);

            for(int f = 0; f < COUNTDOWN_FRAMES; f++)
            {
                bn::core::update();
            }
        }
    }

    void run_game_over(bn::sprite_text_generator& text_gen,
                       int player_score, int opponent_score)
    {
        bn::vector<bn::sprite_ptr, 32> sprites;
        text_gen.set_center_alignment();

        text_gen.generate(0, -20,
                          player_score >= WIN_SCORE ? "YOU WIN!" : "YOU LOSE!",
                          sprites);

        bn::string<24> score;
        bn::ostringstream score_stream(score);
        score_stream << player_score << " - " << opponent_score;
        text_gen.generate(0, 0, score, sprites);
        text_gen.generate(0, 30, "PRESS START", sprites);

        while(true)
        {
            if(bn::keypad::start_pressed())
            {
                return;
            }
            bn::core::update();
        }
    }

    void sync_sprites(Player& player, Opponent& opponent, Ball& ball,
                      bn::sprite_ptr& player_spr, bn::sprite_ptr& opponent_spr,
                      bn::sprite_ptr& ball_spr)
    {
        player_spr.set_position(player.entity.x, player.entity.y);
        opponent_spr.set_position(opponent.entity.x, opponent.entity.y);
        ball_spr.set_position(ball.entity.x, ball.entity.y);
    }
}

void run_game(bn::sprite_text_generator& text_gen, bn::random& rng)
{
    Player player;
    Opponent opponent;
    Ball ball;

    bn::sprite_ptr player_spr = bn::sprite_ptr::create(
        player.entity.x, player.entity.y, bn::sprite_items::player);
    bn::sprite_ptr opponent_spr = bn::sprite_ptr::create(
        opponent.entity.x, opponent.entity.y, bn::sprite_items::opponent);
    bn::sprite_ptr ball_spr = bn::sprite_ptr::create(
        ball.entity.x, ball.entity.y, bn::sprite_items::ball);
    bn::sprite_ptr left_hoop_spr = bn::sprite_ptr::create(
        LEFT_HOOP_X, HOOP_RIM_Y - 8, bn::sprite_items::hoop);
    bn::sprite_ptr right_hoop_spr = bn::sprite_ptr::create(
        RIGHT_HOOP_X, HOOP_RIM_Y - 8, bn::sprite_items::hoop);
    right_hoop_spr.set_horizontal_flip(true);

    int player_score = 0;
    int opponent_score = 0;

    bn::vector<bn::sprite_ptr, 8> score_sprites;
    update_score_display(text_gen, score_sprites, player_score, opponent_score);

    // Initial countdown
    sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);
    run_countdown(text_gen);

    while(true)
    {
        player.update();
        opponent.update(ball.entity, player.entity, rng, opponent_score, player_score);
        ball.update();

        // Blob-vs-blob collision
        {
            constexpr bn::fixed BW = 28;
            constexpr bn::fixed BH = 14;
            bn::fixed dx = player.entity.x - opponent.entity.x;
            bn::fixed dy = player.entity.y - opponent.entity.y;
            bn::fixed overlap_x = BW - abs_fixed(dx);
            bn::fixed overlap_y = BH - abs_fixed(dy);

            if(overlap_x > 0 && overlap_y > 0)
            {
                if(overlap_y < overlap_x)
                {
                    // Vertical resolution — one lands on the other
                    if(dy < 0)
                    {
                        // Player is above opponent
                        player.entity.y = opponent.entity.y - BH;
                        if(player.entity.vy > 0)
                        {
                            player.entity.vy = 0;
                            player.entity.on_ground = true;
                        }
                    }
                    else
                    {
                        // Opponent is above player
                        opponent.entity.y = player.entity.y - BH;
                        if(opponent.entity.vy > 0)
                        {
                            opponent.entity.vy = 0;
                            opponent.entity.on_ground = true;
                        }
                    }
                }
                else
                {
                    // Horizontal push-apart
                    bn::fixed half = overlap_x / 2;
                    if(dx < 0)
                    {
                        player.entity.x -= half;
                        opponent.entity.x += half;
                    }
                    else
                    {
                        player.entity.x += half;
                        opponent.entity.x -= half;
                    }
                }
            }
        }

        // Collisions (32x16 blobs, 8x8 ball)
        if(check_collision(player.entity, 28, 14, ball.entity, 6, 6))
        {
            bn::fixed dir = (ball.entity.x > player.entity.x) ? 1 : -1;
            ball.hit(player.entity, dir);
        }
        if(check_collision(opponent.entity, 28, 14, ball.entity, 6, 6))
        {
            bn::fixed dir = (ball.entity.x > opponent.entity.x) ? 1 : -1;
            ball.hit(opponent.entity, dir);
        }

        // Scoring — ball must pass through the rim
        bool scored = false;
        bool player_scored_point = false;
        if(ball.scored_in_hoop(RIGHT_HOOP_X))
        {
            player_score++;
            player_scored_point = true;
            scored = true;
        }
        else if(ball.scored_in_hoop(LEFT_HOOP_X))
        {
            opponent_score++;
            scored = true;
        }

        if(scored)
        {
            update_score_display(text_gen, score_sprites, player_score, opponent_score);

            if(player_score >= WIN_SCORE || opponent_score >= WIN_SCORE)
            {
                player_spr.set_visible(false);
                opponent_spr.set_visible(false);
                ball_spr.set_visible(false);
                left_hoop_spr.set_visible(false);
                right_hoop_spr.set_visible(false);
                score_sprites.clear();

                run_game_over(text_gen, player_score, opponent_score);
                return;
            }

            // Reset positions — scored-on side gets possession
            player.reset_position();
            opponent.reset_position();
            ball.reset(!player_scored_point); // give ball to the team that was scored on
            sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);

            run_countdown(text_gen);
        }

        sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);
        bn::core::update();
    }
}

}
