#ifndef __DOOR_CONTAINER_H__
#define __DOOR_CONTAINER_H__

#include <fb/game/door.h>

namespace fb::game {

class door_container : private std::unordered_map<uint64_t, std::shared_ptr<fb::game::door>>
{
private:
    using super = std::unordered_map<uint64_t, std::shared_ptr<fb::game::door>>;

public:
    class iterator;
    class const_iterator;

    using base_iterator       = super::iterator;
    using const_base_iterator = super::const_iterator;

    friend class iterator;
    friend class const_iterator;

public:
    const fb::game::map& map;

public:
    using super::at;
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::contains;
    using super::end;
    using super::size;

public:
    door_container(const fb::game::map& map);
    door_container(const door_container&) = delete;
    ~door_container();

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    void           add(std::shared_ptr<door>& door);
    door*          find(const character& ch) const;
};

class door_container::iterator : public door_container::base_iterator
{
public:
    std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    iterator(const door_container::base_iterator& i, const door_container& container);
    ~iterator() = default;

public:
    std::pair<fb::model::point16_t, door&> operator* ();
};

class door_container::const_iterator : public door_container::const_base_iterator
{
public:
    const std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    const_iterator(const door_container::const_base_iterator& i, const door_container& container);
    ~const_iterator() = default;

public:
    const std::pair<fb::model::point16_t, door&> operator* () const;
};

} // namespace fb::game

#endif