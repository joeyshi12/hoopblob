#include "menu_scene.h"
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_sound_items.h"
#include "bn_regular_bg_items_menu_bg.h"

namespace hb
{

namespace
{
    void draw_menu(bn::sprite_text_generator& text_gen,
                   bn::vector<bn::sprite_ptr, 32>& sprites, int cursor)
    {
        sprites.clear();
        text_gen.set_center_alignment();
        text_gen.generate(0, -40, "HOOPBLOB", sprites);
        text_gen.generate(0, 0, cursor == 0 ? "> START" : "  START", sprites);
        text_gen.generate(0, 16, cursor == 1 ? "> CREDITS" : "  CREDITS", sprites);
    }
}

SceneType run_menu(bn::sprite_text_generator& text_gen)
{
    bn::regular_bg_ptr bg = bn::regular_bg_items::menu_bg.create_bg(0, 0);
    bn::vector<bn::sprite_ptr, 32> sprites;
    int cursor = 0;
    draw_menu(text_gen, sprites, cursor);

    while(true)
    {
        if(bn::keypad::down_pressed() && cursor == 0)
        {
            cursor = 1;
            draw_menu(text_gen, sprites, cursor);
        }
        else if(bn::keypad::up_pressed() && cursor == 1)
        {
            cursor = 0;
            draw_menu(text_gen, sprites, cursor);
        }

        if(bn::keypad::a_pressed())
        {
            bn::sound_items::sfx_menu_select.play();
            return cursor == 0 ? SceneType::GAME : SceneType::CREDITS;
        }

        bn::core::update();
    }
}

}
