#include <fb/game/item.h>
#include <fb/game/context.h>

fb::game::weapon::weapon(fb::game::context& context, const fb::model::weapon& model) : 
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
    auto&                   model = this->based<fb::model::weapon>();
    
    sstream << "파괴력: 　　 S:　" << std::to_string(model.damage_small.min) << 'm' << std::to_string(model.damage_small.max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(model.damage_large.min) << 'm' << std::to_string(model.damage_large.max) << std::endl;
    return sstream.str();
}

std::string fb::game::weapon::vname(NAME_OPTION option) const
{
    auto                    sstream = std::stringstream();
    auto&                   model = this->based<fb::model::equipment>();
    sstream << this->_custom_name.value_or(model.name);

    switch(option)
    {
        case NAME_OPTION::TRADE:
        {
            auto percent = this->_durability / float(model.durability) * 100;;
            sstream << '(' 
                    << std::fixed << std::setprecision(1) << percent 
                    << "%)";
        }
        break;
    }
    
    return sstream.str();
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