#include <item.h>
#include <character.h>

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
    
    auto listener = this->_owner->get_listener<fb::game::character>();
    if(listener != nullptr)
    {
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));
        listener->on_action(*this->_owner, ACTION::EAT, DURATION::EAT, static_cast<uint8_t>(SOUND::EAT));
    }

    if(this->empty())
        this->_owner->items.remove(*this, -1, ITEM_DELETE_TYPE::EAT);
    return true;
}