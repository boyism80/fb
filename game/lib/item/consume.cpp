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
    if (this->_container == nullptr)
        return false;

    auto owner = this->_container->owner();
    if (owner == nullptr)
        return false;

    if (this->_count == 0)
        return false;

    fb::game::item::active();
    this->_count--;

    owner->action(ACTION::EAT, DURATION::EAT);
    owner->sound(SOUND::EAT);
    owner->listener.on_item_update(*owner, owner->items.index(this->shared_from_this_as<fb::game::item>()));

    if (this->empty())
        std::ignore = owner->items.remove(this->shared_from_this_as<fb::game::item>(), -1, ITEM_DELETE_TYPE::EAT);

    return true;
}