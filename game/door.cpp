#include "door.h"
#include "map.h"

bool fb::game::door::core::matched(const map& map, const point16_t& position, bool open) const
{
    auto door_size = this->size();
    for(int i = 0; i < door_size; i++)
    {
        auto now = map(position.x + i, position.y);
        if(now == nullptr)
            return false;

        auto dst = open ? this->at(i).open : this->at(i).close;

        if(now->object != dst)
            return false;
    }

    return true;
}

bool fb::game::door::core::find(const map& map, point16_t& position, bool open) const
{
    bool init = true;
    for(int offset = position.y * map.width() + position.x; offset < map.width() * map.height(); offset++)
    {
        auto row = offset / map.width();
        auto col = offset % map.width();

        auto pos = point16_t(col, row);
        if(this->matched(map, pos, open) == false)
            continue;

        position = pos;
        return true;
    }

    return false;
}

fb::game::door::door(fb::game::map* owner, fb::game::door::core& core, const point16_t position, bool opened) : 
    _owner(owner),
    _core(core), 
    position(position), 
    _opened(opened)
{
}

fb::game::door::~door()
{
}

const fb::game::door::core& fb::game::door::based() const
{
    return this->_core;
}

bool fb::game::door::toggle()
{
    this->_opened = !this->_opened;

    for(int i = 0; i < this->_core.size(); i++)
    {
        auto& object = (*this->_owner)(this->position.x + i, this->position.y)->object;
        auto  door_e = this->_opened ? this->_core[i].open : this->_core[i].close;

        object = door_e;
    }

    return true;
}

bool fb::game::door::opened() const
{
    return this->_opened;
}

fb::game::doors::doors()
{
}

fb::game::doors::~doors()
{
    for(auto door : *this)
        delete door;
}

void fb::game::doors::add(map* map, fb::game::door::core& core, const point16_t position, bool opened)
{
    this->push_back(new door(map, core, position, opened));
}

fb::game::door* fb::game::doors::find(const point16_t position)
{
    for(auto door : *this)
    {
        const auto& core = door->based();
        for(int i = 0; i < core.size(); i++)
        {
            if(position == point16_t(door->position.x + i, door->position.y))
                return door;
        }
    }
}