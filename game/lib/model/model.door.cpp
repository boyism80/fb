#include <context.h>
#include <map.h>
#include <model.h>

bool fb::model::door::matched(const fb::game::map& map, const point16_t& position, bool is_open) const
{
    for (int i = 0, n = this->width; i < n; i++)
    {
        auto tile = map(position.x + i, position.y);
        if (tile == nullptr)
            return false;

        auto& pair  = map.context.model.door_pair[this->pairs[i]];
        auto  value = is_open ? pair.open : pair.close;
        if (tile->object != value)
            return false;
    }
    return true;
}

bool fb::model::door::find(const fb::game::map& map, point16_t& position, bool is_open) const
{
    auto x     = position.x / map.width();
    auto y     = position.y + (position.x % map.width());
    auto begin = point16_t(x, y);
    auto end   = point16_t(map.width() - 1, map.height() - 1);
    for (uint64_t i = map.index(begin), n = map.index(end); i <= n; i++)
    {
        if (this->matched(map, map.point(i), is_open))
        {
            position = map.point(i);
            return true;
        }
    }
    return false;
}