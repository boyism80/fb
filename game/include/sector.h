#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <object.h>
#include <set>

using namespace fb::model;

namespace fb { namespace game {

class map;
class character;
class sectors;

class sector : private std::vector<fb::game::object*>
{
public:
    friend class sectors;

public:
    using std::vector<fb::game::object*>::begin;
    using std::vector<fb::game::object*>::cbegin;
    using std::vector<fb::game::object*>::end;
    using std::vector<fb::game::object*>::cend;
    using std::vector<fb::game::object*>::erase;
    using std::vector<fb::game::object*>::at;
    using std::vector<fb::game::object*>::operator[];
    using std::vector<fb::game::object*>::size;

private:
    const uint32_t               _id        = 0;
    bool                         _activated = false;
    std::function<void(sector&)> _state_changed;

public:
    sector(uint32_t id, const std::function<void(sector&)>& state_changed);
    ~sector();

public:
    void     push(fb::game::object& object);
    void     erase(fb::game::object& object);
    uint32_t id() const;
    bool     activated() const;
};

class sectors
{
public:
    using unique_sectors = std::vector<std::unique_ptr<sector>>;

private:
    const size16_t              _map_size = size16_t(0, 0);
    const size16_t              _size     = size16_t(0, 0);
    const uint32_t              _rows = 0, _columns = 0;
    const uint32_t              _count = 0;
    unique_sectors              _pool;
    std::map<uint32_t, sector*> _activated_cache;

public:
    sectors(const size16_t& map_size, const size16_t& size);
    sectors(const sectors&) = delete;
    sectors(sectors&&)      = delete;
    ~sectors()              = default;

public:
    sectors& operator= (sectors&)       = delete;
    sectors& operator= (const sectors&) = delete;

private:
    uint32_t          index(const point16_t& position) const;
    std::set<sector*> activated_sectors() const;

public:
    sector*  at(const point16_t& position) const;
    sector*  at(uint32_t index) const;
    uint32_t push(fb::game::object& object);

public:
    std::vector<sector*> nears(uint32_t index) const;
    std::vector<sector*> nears(const point16_t& pivot) const;
    std::vector<object*> objects(const point16_t& pivot, OBJECT_TYPE type) const;
    std::vector<object*> activated_objects(OBJECT_TYPE type) const;
    bool                 activated() const;
};

}} // namespace fb::game

#endif // !__SECTOR_H__
