#include <fb/game/context.h>
#include <fb/game/map.h>
#include <fb/model/model.h>

bool fb::model::door::matched(const fb::game::map& map, const point16_t& position, bool is_open) const
{
    for (int i = 0, n = this->width; i < n; i++)
    {
        auto tile = map(position.x + i, position.y);
        if (tile == nullptr)
            return false;

        auto& pair  = map.context.model.door_pair[this->pairs[i]];
        auto  value = is_open ? pair.open : pair.close;
        fb::logger::info("map : {} / x : {} / y : {}", map.model.id, position.x, position.y);
        if (tile->object != value)
            return false;
    }
    return true;
}

bool fb::model::door::matched(const fb::game::map& map, const point16_t& position, bool* opened) const
{
    if (this->matched(map, position, true))
    {
        if (opened != nullptr)
            *opened = true;
        return true;
    }

    if (this->matched(map, position, false))
    {
        if (opened != nullptr)
            *opened = false;
        return true;
    }

    return false;
}