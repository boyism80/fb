#ifndef __DOOR_H__
#define __DOOR_H__

#include <fb/lua.h>
#include <fb/model/model.h>

namespace fb::game {

class map;
class character;

class door : public lua::luable
{
public:
    LUA_PROTOTYPE

    class container;

public:
    class model;

private:
    bool     _opened   = false;
    bool     _locked   = false;
    uint32_t _model_id = 0;

public:
    const fb::game::map&       map;
    const fb::model::point16_t pivot;
    const uint16_t             width;

public:
    door(const fb::game::map& map, const fb::model::door& model, const fb::model::point16_t& pivot, bool opened);
    door(const door&) = delete;
    ~door();

public:
    const fb::model::door& model() const;
    bool                   toggle();
    bool                   opened() const;
    bool                   locked() const;
    void                   lock(bool value);
};

class door::container : private std::unordered_map<uint64_t, std::shared_ptr<fb::game::door>>
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
    container(const fb::game::map& map);
    container(const container&) = delete;
    ~container();

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    void           add(std::shared_ptr<door>& door);
    door*          find(const character& ch) const;
};

class door::container::iterator : public door::container::base_iterator
{
public:
    std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    iterator(const container::base_iterator& i, const container& container);
    ~iterator() = default;

public:
    std::pair<fb::model::point16_t, door&> operator* ();
};

class door::container::const_iterator : public door::container::const_base_iterator
{
public:
    const std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    const_iterator(const container::const_base_iterator& i, const container& container);
    ~const_iterator() = default;

public:
    const std::pair<fb::model::point16_t, door&> operator* () const;
};

} // namespace fb::game

#endif // !__DOOR_H__