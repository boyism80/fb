#include <fb/game/server.h>
#include <fb/game/item.h>

fb::game::weapon::weapon(fb::game::server& server, const fb::model::weapon& model) :
    equipment(server, model)
{ }

fb::game::weapon::weapon(const weapon& right) :
    equipment(right)
{ }

fb::game::weapon::~weapon()
{ }

std::string fb::game::weapon::mid_message() const
{
    std::stringstream sstream;
    auto&             model = this->based<fb::model::weapon>();

    sstream << "파괴력: 　　 S:　" << std::to_string(model.damage_small.min) << 'm'
            << std::to_string(model.damage_small.max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(model.damage_large.min) << 'm'
            << std::to_string(model.damage_large.max) << std::endl;
    return sstream.str();
}

const std::string& fb::game::weapon::name() const
{
    if (this->_custom_name.has_value())
        return this->_custom_name.value();
    else
        return this->_model.name;
}

std::string fb::game::weapon::inven_name() const
{
    auto& model = this->based<fb::model::equipment>();
    return this->_custom_name.value_or(model.name);
}

std::string fb::game::weapon::trade_name() const
{
    auto  sstream    = std::stringstream();
    auto& model      = this->based<fb::model::equipment>();
    float percentage = this->_durability / float(model.durability) * 100;

    sstream << this->_custom_name.value_or(model.name) << '(' << std::fixed << std::setprecision(1) << percentage
            << "%)";

    return sstream.str();
}

const std::optional<std::string>& fb::game::weapon::custom_name() const
{
    return this->_custom_name;
}

void fb::game::weapon::custom_name(std::string_view name)
{
    this->_custom_name = std::string(name);

    if (this->_container == nullptr)
        return;

    auto owner = this->_container->owner();
    if (owner == nullptr)
        return;

    auto index = this->_container->index(this->shared_from_this_as<fb::game::item>());
    if (index != 0xFF && owner != nullptr)
        owner->listener.on_item_update(*owner, index);
}

void fb::game::weapon::reset_custom_name()
{
    auto owner = this->_container->owner();
    if (owner == nullptr)
        return;

    if (this->_container == nullptr)
        return;

    this->_custom_name.reset();

    auto index = this->_container->index(this->shared_from_this_as<fb::game::item>());
    if (index != 0xFF)
        owner->listener.on_item_update(*owner, index);
}

fb::protocol::internal::Item fb::game::weapon::to_protocol(EQUIPMENT_PARTS parts) const
{
    auto base        = fb::game::item::to_protocol(parts);
    base.custom_name = this->_custom_name;
    return base;
}