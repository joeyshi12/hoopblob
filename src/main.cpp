#include "bn_core.h"
#include "bn_random.h"
#include "bn_sprite_font.h"
#include "bn_sprite_text_generator.h"

#include "scene.h"
#include "menu_scene.h"
#include "credits_scene.h"
#include "game_scene.h"

#include "bn_sprite_items_font.h"

int main()
{
    bn::core::init();

    bn::sprite_font font(bn::sprite_items::font);
    bn::sprite_text_generator text_gen(font);
    text_gen.set_center_alignment();
    bn::random rng;

    hb::SceneType scene = hb::SceneType::MENU;

    while(true)
    {
        switch(scene)
        {
        case hb::SceneType::MENU:
            scene = hb::run_menu(text_gen);
            break;
        case hb::SceneType::CREDITS:
            hb::run_credits(text_gen);
            scene = hb::SceneType::MENU;
            break;
        case hb::SceneType::GAME:
            hb::run_game(text_gen, rng);
            scene = hb::SceneType::MENU;
            break;
        default:
            break;
        }
    }
}
