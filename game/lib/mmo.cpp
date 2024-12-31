#include <fb/game/mmo.h>

using namespace fb::game;

fb::game::ability::ability(uint8_t  strength,
                           uint8_t  intelligence,
                           uint8_t  dexteritry,
                           uint32_t base_hp,
                           uint32_t base_mp,
                           uint32_t exp) :
    strength(strength),
    intelligence(intelligence),
    dexteritry(dexteritry),
    base_hp(base_hp),
    base_mp(base_mp),
    exp(exp)
{ }

fb::game::ability::ability(const ability& right) :
    strength(right.strength),
    intelligence(right.intelligence),
    dexteritry(right.dexteritry),
    base_hp(right.base_hp),
    base_mp(right.base_mp),
    exp(right.exp)
{ }

fb::game::ability::~ability()
{ }

fb::game::class_data::class_data()
{ }

fb::game::class_data::~class_data()
{ }

void fb::game::class_data::push(const ability& ability)
{
    this->abilities.push_back(std::make_unique<fb::game::ability>(ability));
}

void fb::game::class_data::push(const std::string& name)
{
    this->promotions.push_back(name);
}