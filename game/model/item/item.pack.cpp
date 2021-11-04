#include "model/item/item.pack.h"
#include "model/acceptor/acceptor.game.h"

fb::game::pack::master::master(const std::string& name,
                               uint16_t look,
                               uint8_t color,
                               uint32_t id,
                               uint32_t price,
                               const fb::game::item::conditions& condition,
                               penalties penalty,
                               uint16_t capacity,
                               const fb::game::item::trade& trade,
                               const fb::game::item::storage& storage,
                               std::string desc,
                               std::string active_script,
                               uint16_t durability) : fb::game::item::master(name,
                                                                             look, 
                                                                             color,
                                                                             id,
                                                                             price,
                                                                             condition,
                                                                             penalty,
                                                                             capacity,
                                                                             trade,
                                                                             storage,
                                                                             desc,
                                                                             active_script),
    durability(durability)
{ }

fb::game::pack::master::~master()
{ }

fb::game::item::attrs fb::game::pack::master::attr() const
{
    return item::attrs::PACK;
}

fb::game::item* fb::game::pack::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::pack(this, listener);
}

fb::game::pack::pack(const fb::game::pack::master* master, listener* listener) : 
    fb::game::item(master, listener)
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
    if(listener != nullptr)
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));

    if(this->empty())
        this->_owner->items.remove(*this, 0xFF, item::delete_attr::REDUCE);

    return true;
}