#include "model/mmo/mmo.h"
#include "model/session/session.h"

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

fb::ostream fb::game::message::make_stream(const std::string& message, message::type types)
{
    fb::ostream             ostream;
    ostream.write_u8(0x0A)
        .write_u8(types)
        .write(message, true);

    return ostream;
}

fb::ostream fb::game::timer::make_stream(uint32_t time, type type)
{
    fb::ostream             ostream;

    ostream.write_u8(0x67)
        .write_u8(type)
        .write_u32(time)
        .write_u8(0x00);

    return ostream;
}

fb::ostream fb::game::weather::make_stream(type weather)
{
    fb::ostream             ostream;

    ostream.write_u8(0x1F)
        .write_u8(weather)
        .write_u8(0x00);

    return ostream;
}

fb::game::legend_container::legend_container()
{
}

fb::game::legend_container::~legend_container()
{
}

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
