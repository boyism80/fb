#include <door.h>
#include <map.h>
#include <session.h>
#include <context.h>

fb::game::door::door(const fb::game::map& map, const fb::model::door& model, const point16_t& position, bool opened) : 
    map(map),
    model(model), 
    position(position), 
    _opened(opened),
    width(static_cast<uint16_t>(model.pairs.size()))
{ }

fb::game::door::~door()
{ }

bool fb::game::door::toggle()
{
    for (int i = 0; i < this->width; i++)
    {
        auto tile = this->map(this->position.x + i, this->position.y);
        if (tile == nullptr)
            return false;

        auto index = this->model.pairs[i];
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

fb::game::doors::doors(const fb::game::map& map) : map(map)
{ }

fb::game::doors::~doors()
{ }

doors::iterator doors::begin()
{
    return fb::game::doors::iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::begin(), *this);
}

doors::iterator doors::end()
{
   return doors::iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::end(), *this);
}

doors::const_iterator doors::begin() const
{
   return doors::const_iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::cbegin(), *this);
}

doors::const_iterator doors::end() const
{
   return doors::const_iterator(std::unordered_map<uint64_t, std::unique_ptr<door>>::cend(), *this);
}

void fb::game::doors::add(const point16_t& position, const fb::model::door& model, bool opened)
{
    auto index = this->map.index(position);
    std::unordered_map<uint64_t, std::unique_ptr<door>>::insert({index, std::make_unique<fb::game::door>(this->map, model, position, opened) });
}

fb::game::door* fb::game::doors::find(const point16_t position) const
{
    for (auto& [index, door] : *this)
    {
        for (int i = 0; i <= door.model.width; i++)
        {
            auto door_p = point16_t(position.x + i, position.y);
            auto door_index = this->map.index(door_p);
            if (this->contains(door_index))
                return this->at(door_index).get();
        }
    }
    return nullptr;
}

fb::game::door* fb::game::doors::find(const fb::game::session& session) const
{
    auto direction = session.direction();
    auto position = session.position();
    switch (session.direction())
    {
    
    case DIRECTION::TOP:
        position.y = std::max(0, position.y - 1);
        break;

    case DIRECTION::BOTTOM:
        position.y = std::min(this->map.height() - 1, position.y + 1);
    }

    return this->find(position);
}

fb::game::doors::iterator::iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator& i, const doors& container) : 
    std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator(i),
    pair(i != container.end() ? std::make_optional<std::pair<point16_t, door&>>(container.map.point(i->first), *i->second.get()) : std::nullopt)
{ }

std::pair<point16_t, fb::game::door&> fb::game::doors::iterator::operator * ()
{
    return this->pair.value();
}

fb::game::doors::const_iterator::const_iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator& i, const doors& container) :
    std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator(i),
    pair(i != container.end() ? std::make_optional<std::pair<point16_t, door&>>(container.map.point(i->first), *i->second.get()) : std::nullopt)
{
}

const std::pair<point16_t, fb::game::door&> fb::game::doors::const_iterator::operator * () const
{
    return this->pair.value();
}