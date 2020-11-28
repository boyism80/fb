#include "service.map.h"

fb::game::service::map::map()
{}

fb::game::service::map::~map()
{}

bool fb::game::service::map::existable(fb::game::map & map, const point16_t & position) const
{
    return position.x >= 0 && position.y >= 0 && position.x < map.width() && position.y < map.height();
}

bool fb::game::service::map::movable(fb::game::map& map, const point16_t& position) const
{
    if(this->existable(map, position) == false)
        return false;

    if(map.blocked(position.x, position.y))
        return false;

    for(const auto pair : map.objects)
    {
        if(pair.second->visible() == false)
            continue;

        if(pair.second->type() == fb::game::object::types::ITEM)
            continue;

        if(pair.second->position() == position)
            return false;
    }

    return true;
}

bool fb::game::service::map::movable(fb::game::map& map, const fb::game::object& object, fb::game::direction direction) const
{
    point16_t               position = object.position();

    switch(direction)
    {
    case direction::BOTTOM:
        position.y++;
        break;

    case direction::TOP:
        position.y--;
        break;

    case direction::LEFT:
        position.x--;
        break;

    case direction::RIGHT:
        position.x++;
        break;
    }

    if(this->movable(map, position) == false)
        return false;

    return true;
}

bool fb::game::service::map::movable(fb::game::map& map, fb::game::object& object) const
{
    return this->movable(map, object, object.direction());
}
