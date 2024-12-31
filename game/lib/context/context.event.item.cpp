#include <fb/game/context.h>

using namespace fb::game;

void context::on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr)
{
    this->send(me, fb_resp::item_remove(attr, index, 0), scope::SELF);
}

void context::on_item_update(character& me, uint8_t index)
{
    this->send(me, fb_resp::item_update(me, index), scope::SELF);
}

void context::on_item_swap(character& me, uint8_t src, uint8_t dst)
{
    return;
}

void context::on_item_active(character& me, item& item)
{
    auto thread = lua::get();
    if (thread == nullptr)
        return;

    thread->from(item.based<fb::model::item>().script_active.c_str())
        .func("on_active")
        .pushobject(me)
        .pushobject(item)
        .resume(2);
}

void context::on_item_throws(character& me, item& item, const point16_t& to)
{
    if (me.position() != to)
        this->send(me, fb_resp::item_throws(me, item, to), scope::PIVOT);
    else
        this->send(me, fb_resp::action(me, ACTION::ATTACK, DURATION::THROW), scope::PIVOT);
}