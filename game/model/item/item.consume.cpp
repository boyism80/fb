#include "model/item/item.consume.h"
#include "model/session/session.h"

fb::game::consume::master::master(const fb::game::item::master::config& config) : fb::game::item::master(config)
{ }

fb::game::consume::master::~master()
{ }

fb::game::item::attrs fb::game::consume::master::attr() const
{
    item::attrs attr = item::attrs::CONSUME;
    if(this->capacity > 1)
        attr = item::attrs(attr | item::attrs::BUNDLE);

    return attr;
}


fb::game::consume::consume(fb::game::context& context, const fb::game::consume::master* master, uint16_t count) : 
    fb::game::item(context, master, fb::game::item::config { .count = count })
{ }

fb::game::consume::consume(const consume& right) : 
    fb::game::item(right)
{ }

fb::game::consume::~consume()
{ }

bool fb::game::consume::active()
{
    if(this->_count == 0)
        return false;
    
    this->_count--;
    
    auto listener = this->_owner->get_listener<fb::game::session>();
    listener->on_item_update(*this->_owner, this->_owner->items.index(*this));
    listener->on_action(*this->_owner, fb::game::action::EAT, fb::game::duration::DURATION_EAT, static_cast<uint8_t>(fb::game::sound::type::EAT));

    if(this->empty())
        this->_owner->items.remove(*this, -1, item::delete_attr::EAT);
    return true;
}