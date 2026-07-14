#ifndef __ITEM_LISTENER_H__
#define __ITEM_LISTENER_H__

#include <fb/game/object.h>
#include <fb/game/item/base.h>

namespace fb::game {

struct item::listener_t : public virtual object::listener_t
{
    // clang-format off
    virtual async::task<void> on_item_remove(character& me, uint8_t index, ITEM_DELETE_TYPE attr = ITEM_DELETE_TYPE::NONE) = 0;
    virtual async::task<void> on_item_update(character& me, uint8_t index) = 0;
    virtual async::task<void> on_item_swap(character& me, uint8_t src, uint8_t dst) = 0;
    virtual async::task<void> on_item_active(character& me, item& item) = 0;
    virtual async::task<void> on_item_throws(character& me, item& item, const fb::model::point16_t& to) = 0;
    // clang-format on
};

} // namespace fb::game

#endif // !__ITEM_LISTENER_H__
