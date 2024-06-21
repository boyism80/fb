#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::weapon::model::model(const fb::game::weapon::model::config& config) : 
    fb::game::equipment::model(config),
    damage_range(config.small, config.large),
    sound(config.sound),
    spell(config.spell),
    rename(config.rename)
{ }


fb::game::weapon::model::~model()
{ }

ITEM_ATTRIBUTE fb::game::weapon::model::attr() const
{
    return ITEM_ATTRIBUTE::WEAPON;
}

fb::game::weapon::types fb::game::weapon::model::weapon_type() const
{
    switch(this->look / 10000)
    {
    case 0:
        return fb::game::weapon::types::NORMAL;

    case 1:
        return fb::game::weapon::types::SPEAR;

    case 2:
        return fb::game::weapon::types::BOW;

    case 3:
        return fb::game::weapon::types::FAN;

    default:
        return fb::game::weapon::types::UNKNOWN;
    }
}


fb::game::weapon::weapon(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::weapon::weapon(const weapon& right) : 
    equipment(right)
{ }

fb::game::weapon::~weapon()
{ }

std::string fb::game::weapon::mid_message() const
{
    std::stringstream       sstream;
    auto                    model = this->based<fb::model::weapon>();
    
    sstream << "파괴력: 　　 S:　" << std::to_string(model->damage_range.small.min) << 'm' << std::to_string(model->damage_range.small.max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(model->damage_range.large.min) << 'm' << std::to_string(model->damage_range.large.max) << std::endl;
    return sstream.str();
}

const std::string& fb::game::weapon::name() const
{
    if (this->_custom_name.has_value())
        return this->_custom_name.value();

    return fb::game::item::name();
}

const std::optional<std::string>& fb::game::weapon::custom_name() const
{
    return this->_custom_name;
}

void fb::game::weapon::custom_name(const std::string& name)
{
    this->_custom_name = name;
    auto listener = this->_owner->get_listener<fb::game::session>();
    if (listener != nullptr)
    {
        auto index = this->_owner->items.index(*this);
        if (index != 0xFF)
            listener->on_item_update(*this->_owner, index);
    }
}

void fb::game::weapon::reset_custom_name()
{
    this->_custom_name.reset();
    auto listener = this->_owner->get_listener<fb::game::session>();
    if (listener != nullptr)
    {
        auto index = this->_owner->items.index(*this);
        if (index != 0xFF)
            listener->on_item_update(*this->_owner, index);
    }
}