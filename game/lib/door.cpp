#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/game/door.h>
#include <fb/game/map.h>

fb::game::door::door(const fb::game::map&   map,
                     const fb::model::door& model,
                     const point16_t&       position,
                     const point16_t&       pivot,
                     bool                   opened) :
    map(map),
    model(model),
    position(position),
    pivot(pivot),
    _opened(opened),
    width(static_cast<uint16_t>(model.pairs.size()))
{ }

fb::game::door::~door()
{ }

bool fb::game::door::toggle()
{
    for (int i = 0; i < this->width; i++)
    {
        auto tile = this->map(this->pivot.x + i, this->pivot.y);
        if (tile == nullptr)
            return false;

        auto  index = this->model.pairs[i];
        auto& model = this->map.context.model.door_pair[index];
        if (this->_opened)
            tile->object = model.close;
        else
            tile->object = model.open;
    }

    this->_opened = !this->_opened;
    return true;
}

bool fb::game::door::opened() const
{
    return this->_opened;
}

bool fb::game::door::locked() const
{
    return this->_locked;
}

void fb::game::door::lock(bool value)
{
    this->_locked = value;
}