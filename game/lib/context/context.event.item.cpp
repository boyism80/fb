#include <fb/game/context.h>

using namespace fb::game;

void context::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    me.send(fb_resp::item_remove(attr, index, 0));
}

void context::on_item_update(character& me, uint8_t index)
{
    me.send(fb_resp::item_update(me, index));
}

void context::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    return;
}

void context::on_item_active(character& me, item& item)
{
    auto& model = item.based<fb::model::item>();
    if (model.on_active.empty())
        return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return;

#if defined DEBUG | defined _DEBUG
    lua->load(model.script);
#endif
    lua->func(model.on_active);
    lua->pushobject(me);
    lua->pushobject(item);
    std::ignore = lua->call(2);
}

void context::on_item_throws(character& me, item& item, const fb::model::point16_t& to)
{
    if (me.position() != to)
        this->send(me, fb_resp::item_throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}