#include <fb/game/door.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/game/map.h>

using namespace fb::game;

door::container::container(const fb::game::map& map) :
    map(map)
{ }

door::container::~container()
{ }

door::container::iterator door::container::begin()
{
    return door::container::iterator(super::begin(), *this);
}

door::container::iterator door::container::end()
{
    return door::container::iterator(super::end(), *this);
}

door::container::const_iterator door::container::begin() const
{
    return door::container::const_iterator(super::cbegin(), *this);
}

door::container::const_iterator door::container::end() const
{
    return door::container::const_iterator(super::cend(), *this);
}

void door::container::add(std::shared_ptr<door>& door)
{
    auto pivot = door->pivot;
    for (int i = 0; i < door->width; i++)
    {
        pivot.x    = door->pivot.x + i;
        auto index = this->map.index(pivot);
        super::insert({index, door});
    }
}

fb::game::door* door::container::find(const character& ch) const
{
    auto direction = ch.direction();
    auto position  = ch.position();
    switch (ch.direction())
    {
    case DIRECTION::TOP:
        position.y = std::max(0, position.y - 1);
        break;

    case DIRECTION::BOTTOM:
        position.y = std::min(this->map.height() - 1, position.y + 1);
        break;

    case DIRECTION::LEFT:
        position.x = std::max(0, position.x - 1);
        break;

    case DIRECTION::RIGHT:
        position.x = std::min(this->map.width() - 1, position.x + 1);
        break;
    }

    auto index = this->map.index(position);
    if (!this->contains(index))
        return nullptr;

    return this->at(index).get();
}

door::container::iterator::iterator(const door::container::base_iterator& i, const container& container) :
    door::container::base_iterator(i),
    pair(i != static_cast<const super&>(container).end()
             ? std::make_optional<std::pair<fb::model::point16_t, door&>>(container.map.point(i->first),
                                                                          *i->second.get())
             : std::nullopt)
{ }

std::pair<fb::model::point16_t, fb::game::door&> door::container::iterator::operator* ()
{
    return this->pair.value();
}

door::container::const_iterator::const_iterator(const door::container::const_base_iterator& i,
                                                const container&                            container) :
    door::container::const_base_iterator(i),
    pair(i != static_cast<const super&>(container).end()
             ? std::make_optional<std::pair<fb::model::point16_t, door&>>(container.map.point(i->first),
                                                                          *i->second.get())
             : std::nullopt)
{ }

const std::pair<fb::model::point16_t, fb::game::door&> door::container::const_iterator::operator* () const
{
    return this->pair.value();
}