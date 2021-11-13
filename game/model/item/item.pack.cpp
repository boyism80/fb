#include "model/item/item.pack.h"
#include "model/context/context.game.h"

fb::game::pack::master::master(const fb::game::item::master::config& config) : fb::game::item::master(config),
    durability(config.capacity)
{ }

fb::game::pack::master::~master()
{ }

fb::game::item::attrs fb::game::pack::master::attr() const
{
    return item::attrs::PACK;
}

fb::game::pack::pack(fb::game::context& context, const fb::game::pack::master* master) : 
    fb::game::item(context, master)
{
    this->_durability = master->durability;
}

fb::game::pack::pack(const pack& right) : 
    item(right),
    _durability(right._durability)
{ }

fb::game::pack::~pack()
{ }


std::optional<uint16_t> fb::game::pack::durability() const
{
    return this->_durability;
}

void fb::game::pack::durability(std::optional<uint16_t> value)
{
    this->_durability = value.value_or(0);
}

const std::string fb::game::pack::name_styled() const
{
    std::stringstream sstream;
    sstream << this->name() << " [" << this->_durability << " 잔]";

    return sstream.str();
}

bool fb::game::pack::active()
{
    if(this->_durability <= 0)
        return false;

    this->_durability--;
    if(this->_durability <= 0)
        this->count(0);
    
    auto listener = this->_owner->get_listener<fb::game::session>();
    listener->on_item_update(*this->_owner, this->_owner->items.index(*this));

    if(this->empty())
        this->_owner->items.remove(*this, 0xFF, item::delete_attr::REDUCE);

    return true;
}