#include <fb/game/character.h>
#include <fb/game/dialog.h>

fb::game::dialog::preset::preset(const character& ch) :
    sex(ch.sex()),
    state(ch.state()),
    face(ch.look()),
    hair_color(ch.color()),
    armor_color(ch.armor_color())
{
    auto ch_weapon = ch.items.weapon();
    if (ch_weapon != nullptr)
    {
        auto& model  = ch_weapon->based<fb::model::weapon>();
        this->weapon = model.dress;
    }

    auto ch_armor = ch.items.armor();
    if (ch_armor != nullptr)
    {
        auto& model = ch_armor->based<fb::model::armor>();
        this->armor = model.dress;
    }

    auto ch_shield = ch.items.shield();
    if (ch_shield != nullptr)
    {
        auto& model  = ch_shield->based<fb::model::shield>();
        this->shield = model.dress;
    }
}