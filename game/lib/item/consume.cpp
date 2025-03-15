#include <fb/game/character.h>
#include <fb/game/item.h>

fb::game::consume::consume(fb::game::context& context, const fb::model::consume& model, uint16_t count) :
    fb::game::item(context, model, initial_params{.count = count})
{ }

fb::game::consume::consume(const consume& right) :
    fb::game::item(right)
{ }

fb::game::consume::~consume()
{ }

bool fb::game::consume::active()
{
    if (this->_count == 0)
        return false;

    fb::game::item::active();
    this->_count--;

    this->_owner->action(ACTION::EAT, DURATION::EAT);
    this->_owner->sound(SOUND::EAT);
    auto listener = this->_owner->get_listener<fb::game::character>();
    if (listener != nullptr)
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));

    if (this->empty())
        std::ignore = this->_owner->items.remove(*this, -1, ITEM_DELETE_TYPE::EAT);

    return true;
}