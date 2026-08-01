#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/door.h>
#include <fb/game/map.h>

using namespace fb::game;
using table = fb::model::table;

door::door(const fb::game::map& map, const fb::model::door& model, const fb::model::point16_t& pivot, bool opened) :
    map(map),
    model(model),
    pivot(pivot),
    _opened(opened),
    width(static_cast<uint16_t>(model.pairs.size()))
{ }

door::~door()
{ }

bool door::toggle()
{
    for (int i = 0; i < this->width; i++)
    {
        auto tile = this->map(this->pivot.x + i, this->pivot.y);
        if (tile == nullptr)
            return false;

        auto  index           = this->model.pairs[i];
        auto  door_pair_table = table::door_pair;
        auto& model           = door_pair_table[index];
        if (this->_opened)
            tile->object = model.close;
        else
            tile->object = model.open;
    }

    this->_opened = !this->_opened;
    return true;
}

bool door::opened() const
{
    return this->_opened;
}

bool door::locked() const
{
    return this->_locked;
}

void door::lock(bool value)
{
    this->_locked = value;
}