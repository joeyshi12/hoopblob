#include "menu_scene.h"
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"

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
            return cursor == 0 ? SceneType::GAME : SceneType::CREDITS;
        }

        bn::core::update();
    }
}

}
