#include <fb/game/item.h>
#include <fb/game/session.h>

fb::game::consume::consume(fb::game::context& context, const fb::model::consume& model, uint16_t count) : 
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
    if(listener != nullptr)
    {
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));
        listener->on_action(*this->_owner, fb::game::ACTION_TYPE::EAT, fb::game::DURATION::EAT, static_cast<uint8_t>(fb::game::SOUND_TYPE::EAT));
    }

    if(this->empty())
        this->_owner->items.remove(*this, -1, DELETE_TYPE::EAT);
    return true;
}