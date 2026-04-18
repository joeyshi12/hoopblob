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
#include "bn_regular_bg_ptr.h"
#include "bn_regular_bg_items_game_bg.h"
#include "bn_sound_items.h"

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
                              bn::vector<bn::sprite_ptr, 16>& sprites,
                              int player_score, int opponent_score,
                              int time_frames, bool overtime)
    {
        sprites.clear();
        text_gen.set_center_alignment();

        // Single consolidated scoreboard: "P  <time>  O" at the top of the screen.
        bn::string<32> line;
        bn::ostringstream s(line);
        s << player_score << "   ";
        if(overtime)
        {
            s << "OT";
        }
        else
        {
            int secs = time_frames / 60;
            int mm = secs / 60;
            int ss = secs % 60;
            s << mm << ":" << (ss < 10 ? "0" : "") << ss;
        }
        s << "   " << opponent_score;
        text_gen.generate(0, -72, line, sprites);
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
            bn::sound_items::sfx_countdown.play();

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
                          player_score > opponent_score ? "YOU WIN!" : "YOU LOSE!",
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

    // Global visual scale applied to all gameplay sprites. Collision sizes
    // below are increased in the same proportion.
    constexpr bn::fixed ENTITY_SCALE = bn::fixed(1.5);

    // Squash when falling fast, stretch when rising fast. Optional `charge` > 0
    // applies a crouch-squish while on the ground (e.g. charging a jump). The
    // sprite is also nudged down so the blob's feet appear to stay planted.
    void apply_blob_squash(bn::sprite_ptr& spr, const Entity& e, bn::fixed charge = 0)
    {
        bn::fixed hs = ENTITY_SCALE;
        bn::fixed vs = ENTITY_SCALE;
        bn::fixed y_offset = 0;
        if(!e.on_ground)
        {
            bn::fixed s = e.vy * bn::fixed(0.05);
            if(s > bn::fixed(0.25)) s = bn::fixed(0.25);
            if(s < bn::fixed(-0.25)) s = bn::fixed(-0.25);
            vs = ENTITY_SCALE * (bn::fixed(1) - s);
            hs = ENTITY_SCALE * (bn::fixed(1) + s * bn::fixed(0.5));
        }
        else if(charge > 0)
        {
            bn::fixed c = charge * bn::fixed(0.3); // up to 30% squish
            vs = ENTITY_SCALE * (bn::fixed(1) - c);
            hs = ENTITY_SCALE * (bn::fixed(1) + c * bn::fixed(0.5));
            y_offset = BLOB_HALF_H * c; // push sprite down so feet stay on floor
        }
        spr.set_scale(hs, vs);
        spr.set_y(e.y + y_offset);
    }

    void face_by_velocity(bn::sprite_ptr& spr, bn::fixed vx, bool default_flip)
    {
        // Base art faces right. default_flip=true for opponent (starts flipped).
        bool flip = default_flip;
        if(vx > bn::fixed(0.2)) flip = false;      // facing right
        else if(vx < bn::fixed(-0.2)) flip = true; // facing left
        spr.set_horizontal_flip(flip);
    }

    // Reset a blob sprite's orientation and affine transform so it starts each
    // round in a clean default state (no lingering squash/flip from prior play).
    void reset_blob_sprite(bn::sprite_ptr& spr, bool default_flip)
    {
        spr.set_scale(ENTITY_SCALE, ENTITY_SCALE);
        spr.set_horizontal_flip(default_flip);
    }
}

void run_game(bn::sprite_text_generator& text_gen, bn::random& rng)
{
    bn::regular_bg_ptr bg = bn::regular_bg_items::game_bg.create_bg(0, 0);

    Player player;
    Opponent opponent;
    Ball ball;

    bn::sprite_ptr player_spr = bn::sprite_ptr::create(
        player.entity.x, player.entity.y, bn::sprite_items::player);
    bn::sprite_ptr opponent_spr = bn::sprite_ptr::create(
        opponent.entity.x, opponent.entity.y, bn::sprite_items::opponent);
    opponent_spr.set_horizontal_flip(true); // opponent faces left (toward player) by default
    bn::sprite_ptr ball_spr = bn::sprite_ptr::create(
        ball.entity.x, ball.entity.y, bn::sprite_items::ball);
    ball_spr.set_scale(ENTITY_SCALE);
    bn::sprite_ptr left_hoop_spr = bn::sprite_ptr::create(
        LEFT_HOOP_X, HOOP_RIM_Y - 3, bn::sprite_items::hoop);
    left_hoop_spr.set_scale(ENTITY_SCALE);
    bn::sprite_ptr right_hoop_spr = bn::sprite_ptr::create(
        RIGHT_HOOP_X, HOOP_RIM_Y - 3, bn::sprite_items::hoop);
    right_hoop_spr.set_scale(ENTITY_SCALE);
    right_hoop_spr.set_horizontal_flip(true);

    int player_score = 0;
    int opponent_score = 0;
    int time_left = MATCH_FRAMES;
    bool overtime = false;
    bool player_was_overlapping = false;
    bool opponent_was_overlapping = false;
    int player_hit_cooldown = 0;
    int opponent_hit_cooldown = 0;

    bn::vector<bn::sprite_ptr, 16> score_sprites;
    update_score_display(text_gen, score_sprites, player_score, opponent_score, time_left, overtime);

    // Initial countdown
    sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);
    reset_blob_sprite(player_spr, false);
    reset_blob_sprite(opponent_spr, true);
    run_countdown(text_gen);

    while(true)
    {
        if(bn::keypad::start_pressed())
        {
            bn::vector<bn::sprite_ptr, 16> pause_sprites;
            text_gen.set_center_alignment();
            int pcursor = 0;
            auto draw_pause = [&]{
                pause_sprites.clear();
                text_gen.generate(0, -20, "PAUSED", pause_sprites);
                text_gen.generate(0, 4, pcursor == 0 ? "> RESUME" : "  RESUME", pause_sprites);
                text_gen.generate(0, 20, pcursor == 1 ? "> QUIT" : "  QUIT", pause_sprites);
            };
            draw_pause();
            while(true)
            {
                bn::core::update();
                if(bn::keypad::down_pressed() && pcursor == 0) { pcursor = 1; draw_pause(); }
                else if(bn::keypad::up_pressed() && pcursor == 1) { pcursor = 0; draw_pause(); }
                if(bn::keypad::a_pressed() || bn::keypad::start_pressed())
                {
                    bn::sound_items::sfx_menu_select.play();
                    if(pcursor == 1)
                    {
                        bn::core::update(); // consume A/START so the main menu doesn't re-trigger
                        return;
                    }
                    break;
                }
            }
        }

        player.update();
        opponent.update(ball.entity, player.entity, rng, opponent_score, player_score);
        BallEvents ball_events = ball.update();
        if(ball_events.rim_hit)
        {
            bn::sound_items::sfx_rim.play();
        }
        else if(ball_events.bounced)
        {
            bn::sound_items::sfx_bounce.play();
        }

        // Blob-vs-blob collision
        {
            constexpr bn::fixed BW = 42;
            constexpr bn::fixed BH = 21;
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

        // Collisions (32x16 blobs, 8x8 ball). Track per-hitter overlap so the
        // hit SFX only plays once per contact (prevents rapid-fire when a blob
        // sits inside the ball).
        if(player_hit_cooldown > 0) player_hit_cooldown--;
        if(opponent_hit_cooldown > 0) opponent_hit_cooldown--;

        bool player_overlap = check_collision(player.entity, 42, 21, ball.entity, 9, 9);
        if(player_overlap)
        {
            if(!player_was_overlapping && player_hit_cooldown == 0)
            {
                bn::fixed dir = (ball.entity.x > player.entity.x) ? 1 : -1;
                ball.hit(player.entity, dir, player.jump_power());
                bn::sound_items::sfx_hit.play();
                player_hit_cooldown = 8;
            }
        }
        player_was_overlapping = player_overlap;

        bool opponent_overlap = check_collision(opponent.entity, 42, 21, ball.entity, 9, 9);
        if(opponent_overlap)
        {
            if(!opponent_was_overlapping && opponent_hit_cooldown == 0)
            {
                bn::fixed dir = (ball.entity.x > opponent.entity.x) ? 1 : -1;
                ball.hit(opponent.entity, dir);
                bn::sound_items::sfx_hit.play();
                opponent_hit_cooldown = 8;
            }
        }
        opponent_was_overlapping = opponent_overlap;

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
            bn::sound_items::sfx_score.play();
            update_score_display(text_gen, score_sprites, player_score, opponent_score, time_left, overtime);

            if(overtime)
            {
                // Sudden death — first score ends the match.
                bn::sound_items::sfx_game_over.play();
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
            player_was_overlapping = false;
            opponent_was_overlapping = false;
            player_hit_cooldown = 0;
            opponent_hit_cooldown = 0;
            sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);
            reset_blob_sprite(player_spr, false);
            reset_blob_sprite(opponent_spr, true);

            run_countdown(text_gen);
        }

        sync_sprites(player, opponent, ball, player_spr, opponent_spr, ball_spr);
        apply_blob_squash(player_spr, player.entity, player.charge_progress());
        apply_blob_squash(opponent_spr, opponent.entity, opponent.charge_progress());
        // Face the direction of movement (not forced to face the ball).
        // Player facing: only change when a direction is actively pressed.
        // If no movement keys are held, hold the last facing (do not snap to default).
        if(bn::keypad::left_held())      player_spr.set_horizontal_flip(true);
        else if(bn::keypad::right_held()) player_spr.set_horizontal_flip(false);
        face_by_velocity(opponent_spr, opponent.entity.vx, true);

        // Match timer
        if(!overtime)
        {
            if(time_left > 0)
            {
                // Refresh HUD once per second to avoid rebuilding sprites every frame.
                int before = time_left;
                time_left--;
                if((before / 60) != (time_left / 60))
                {
                    update_score_display(text_gen, score_sprites, player_score, opponent_score, time_left, overtime);
                }
            }
            else
            {
                if(player_score == opponent_score)
                {
                    overtime = true;
                    update_score_display(text_gen, score_sprites, player_score, opponent_score, time_left, overtime);
                }
                else
                {
                    bn::sound_items::sfx_game_over.play();
                    player_spr.set_visible(false);
                    opponent_spr.set_visible(false);
                    ball_spr.set_visible(false);
                    left_hoop_spr.set_visible(false);
                    right_hoop_spr.set_visible(false);
                    score_sprites.clear();
                    run_game_over(text_gen, player_score, opponent_score);
                    return;
                }
            }
        }

        bn::core::update();
    }
}

}
