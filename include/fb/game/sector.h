#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <fb/game/object.h>
#include <set>

namespace fb::game {

class map;
class character;
class sectors;

class sector : private std::vector<std::shared_ptr<object>>
{
private:
    using super = std::vector<std::shared_ptr<object>>;

public:
    friend class sectors;

public:
    using super::at;
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::end;
    using super::erase;
    using super::operator[];
    using super::size;

private:
    const uint32_t _id              = 0;
    uint32_t       _character_count = 0;
    bool           _activated       = false;

public:
    sector(uint32_t id);
    ~sector();

public:
    void     push(std::shared_ptr<object> object);
    void     erase(std::shared_ptr<object> object);
    uint32_t id() const;
    bool     is_active() const;
};

class sectors
{
public:
    using shared_sectors = std::vector<std::shared_ptr<sector>>;

private:
    const fb::model::size16_t _map_size = fb::model::size16_t(0, 0);
    const fb::model::size16_t _size     = fb::model::size16_t(0, 0);
    const uint32_t            _rows     = 0;
    const uint32_t            _columns  = 0;
    const uint32_t            _count    = 0;
    shared_sectors            _pool;

public:
    sectors(const fb::model::size16_t& map_size, const fb::model::size16_t& size);
    sectors(const sectors&) = delete;
    sectors(sectors&&)      = delete;
    ~sectors()              = default;

public:
    sectors& operator= (sectors& other)       = delete;
    sectors& operator= (const sectors& other) = delete;

private:
    uint32_t                          index(const fb::model::point16_t& position) const;
    std::set<std::shared_ptr<sector>> active_sectors() const;

public:
    std::shared_ptr<sector> at(const fb::model::point16_t& position) const;
    std::shared_ptr<sector> at(uint32_t index) const;

public:
    std::vector<std::shared_ptr<sector>> nears(uint32_t index) const;
    std::vector<std::shared_ptr<sector>> nears(const fb::model::point16_t& pivot) const;
    std::vector<std::shared_ptr<object>> objects(const fb::model::point16_t& pivot, OBJECT_TYPE type) const;
    std::vector<std::shared_ptr<object>> objects(OBJECT_TYPE type) const;
    bool                                 is_active() const;
};

} // namespace fb::game

#endif // !__SECTOR_H__
