#include "credits_scene.h"
#include "bn_core.h"
#include "bn_keypad.h"
#include "bn_vector.h"
#include "bn_sprite_ptr.h"

namespace hb
{

void run_credits(bn::sprite_text_generator& text_gen)
{
    bn::vector<bn::sprite_ptr, 32> sprites;
    text_gen.set_center_alignment();
    text_gen.generate(0, -30, "CREDITS", sprites);
    text_gen.generate(0, 0, "MADE BY", sprites);
    text_gen.generate(0, 14, "JOEY SHI", sprites);
    text_gen.generate(0, 50, "PRESS B TO GO BACK", sprites);

    while(true)
    {
        if(bn::keypad::b_pressed())
        {
            return;
        }
        bn::core::update();
    }
}

}
