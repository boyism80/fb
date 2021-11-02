#include "model/mmo/mmo.h"

using namespace fb::game;

fb::game::level_ability::level_ability(uint8_t strength, uint8_t intelligence, uint8_t dexteritry, uint32_t base_hp, uint32_t base_mp, uint32_t exp) : 
    strength(strength), intelligence(intelligence), dexteritry(dexteritry), base_hp(base_hp), base_mp(base_mp), exp(exp)
{ }

fb::game::level_ability::~level_ability()
{ }

fb::game::class_data::class_data()
{ }

fb::game::class_data::~class_data()
{
    for(auto i : this->level_abilities)
        delete i;
}

void fb::game::class_data::add_level_ability(level_ability* data)
{
    this->level_abilities.push_back(data);
}

void fb::game::class_data::add_promotion(const std::string& name)
{
    this->promotions.push_back(name);
}

fb::game::legend_container::legend_container()
{ }

fb::game::legend_container::~legend_container()
{ }

legend_container& fb::game::legend_container::push(const legend& legend)
{
    this->push_back(legend);
    return *this;
}

legend_container& fb::game::legend_container::push(uint8_t look, uint8_t color, const std::string& content)
{
    this->push_back(legend(look, color, content));
    return *this;
}
