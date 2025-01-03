#include <fb/game/door/container.h>
#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/game/map.h>

using namespace fb::game;

door_container::door_container(const fb::game::map& map) :
    map(map)
{ }

door_container::~door_container()
{ }

door_container::iterator door_container::begin()
{
    return door_container::iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::begin(), *this);
}

door_container::iterator door_container::end()
{
    return door_container::iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::end(), *this);
}

door_container::const_iterator door_container::begin() const
{
    return door_container::const_iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::cbegin(), *this);
}

door_container::const_iterator door_container::end() const
{
    return door_container::const_iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::cend(), *this);
}

void door_container::add(const fb::model::point16_t& position,
                         const fb::model::point16_t& pivot,
                         const fb::model::door&      model,
                         bool                        opened)
{
    auto index = this->map.index(position);
    std::unordered_map<uint64_t, std::unique_ptr<door>>::insert(
        {index, std::make_unique<door>(this->map, model, position, pivot, opened)});
}

fb::game::door* door_container::find(const character& ch) const
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
    }

    auto index = this->map.index(position);
    if (!this->contains(index))
        return nullptr;

    return this->at(index).get();
}

door_container::iterator::iterator(const door_container::base_iterator& i, const door_container& container) :
    door_container::base_iterator(i),
    pair(i != static_cast<const std::unordered_map<uint64_t, std::unique_ptr<door>>&>(container).end()
             ? std::make_optional<std::pair<fb::model::point16_t, door&>>(container.map.point(i->first),
                                                                          *i->second.get())
             : std::nullopt)
{ }

std::pair<fb::model::point16_t, fb::game::door&> door_container::iterator::operator* ()
{
    return this->pair.value();
}

door_container::const_iterator::const_iterator(const door_container::const_base_iterator& i,
                                               const door_container&                      container) :
    door_container::const_base_iterator(i),
    pair(i != static_cast<const std::unordered_map<uint64_t, std::unique_ptr<door>>&>(container).end()
             ? std::make_optional<std::pair<fb::model::point16_t, door&>>(container.map.point(i->first),
                                                                          *i->second.get())
             : std::nullopt)
{ }

const std::pair<fb::model::point16_t, fb::game::door&> door_container::const_iterator::operator* () const
{
    return this->pair.value();
}