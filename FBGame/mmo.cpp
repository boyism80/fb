#include "mmo.h"

using namespace fb::game;

fb::game::_level_ability::_level_ability(uint8_t strength, uint8_t intelligence, uint8_t dexteritry, uint32_t base_hp, uint32_t base_mp, uint32_t exp) : 
	strength(strength), intelligence(intelligence), dexteritry(dexteritry), base_hp(base_hp), base_mp(base_mp), exp(exp)
{
}

fb::game::_level_ability::~_level_ability()
{
}

fb::game::class_data::class_data()
{
}

fb::game::class_data::~class_data()
{
	for(auto i = this->level_abilities.begin(); i != this->level_abilities.end(); i++)
		delete *i;
}

void fb::game::class_data::add_level_ability(level_ability* data)
{
	this->level_abilities.push_back(data);
}

void fb::game::class_data::add_promotion(const std::string& name)
{
	this->promotions.push_back(name);
}