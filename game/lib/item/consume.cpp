#include <fb/game/item.h>
#include <fb/game/session.h>

fb::game::consume::model::model(const fb::game::item::model::config& config) : fb::game::item::model(config)
{ }

fb::game::consume::model::~model()
{ }

fb::game::item::attrs fb::game::consume::model::attr() const
{
    item::attrs attr = item::attrs::CONSUME;
    if(this->capacity > 1)
        attr = item::attrs(attr | item::attrs::BUNDLE);

    return attr;
}


fb::game::consume::consume(fb::game::context& context, const fb::game::consume::model* model, uint16_t count) : 
    fb::game::item(context, model, fb::game::item::config { .count = count })
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