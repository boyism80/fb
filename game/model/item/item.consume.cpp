#include "model/item/item.consume.h"
#include "model/session/session.h"

fb::game::consume::master::master(const std::string& name,
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
                               std::string active_script) : fb::game::item::master(name,
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
                                                                                   active_script)
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

fb::game::item* fb::game::consume::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::consume(this, listener);
}



fb::game::consume::consume(const fb::game::consume::master* master, listener* listener, uint16_t count) : 
    fb::game::item(master, listener, count)
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
    if(listener != nullptr)
    {
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));
        listener->on_action(*this->_owner, fb::game::action::EAT, fb::game::duration::DURATION_EAT, static_cast<uint8_t>(fb::game::sound::type::EAT));
    }

    if(this->empty())
        this->_owner->items.remove(*this, -1, item::delete_attr::EAT);
    return true;
}