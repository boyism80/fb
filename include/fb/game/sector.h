#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <fb/game/object.h>
#include <vector>

namespace fb::game {

class map;

class sector : private std::vector<std::shared_ptr<object>>
{
private:
    using super = std::vector<std::shared_ptr<object>>;

public:
    class container;

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

public:
    explicit sector(uint32_t id);
    ~sector();

public:
    void     push(std::shared_ptr<object> object);
    void     erase(std::shared_ptr<object> object);
    uint32_t id() const;
    bool     occupied() const;

private:
    friend class container;
};

class sector::container
{
private:
    fb::game::map&      _map;
    std::vector<sector> _pool;
    fb::model::size16_t _map_size  = fb::model::size16_t(0, 0);
    fb::model::size16_t _cell_size = fb::model::size16_t(0, 0);
    uint32_t            _rows      = 0;
    uint32_t            _columns   = 0;

public:
    using iterator       = std::vector<sector>::iterator;
    using const_iterator = std::vector<sector>::const_iterator;

public:
    explicit container(fb::game::map& map);
    container(const container&)             = delete;
    container(container&&)                  = delete;
    container& operator= (const container&) = delete;
    container& operator= (container&&)      = delete;
    ~container()                            = default;

public:
    iterator       begin();
    iterator       end();
    const_iterator begin() const;
    const_iterator end() const;
    size_t         size() const;

public:
    bool loaded() const;
    void load(const fb::model::size16_t& map_size, const fb::model::size16_t& cell_size);

    bool                       activated(uint32_t index) const;
    bool                       activated(const sector& sector) const;
    bool                       any_occupied() const;
    std::vector<const sector*> actives() const;

    sector*       at(const fb::model::point16_t& position);
    const sector* at(const fb::model::point16_t& position) const;
    sector*       at(uint32_t index);
    const sector* at(uint32_t index) const;

    std::vector<const sector*>           nears(uint32_t index) const;
    std::vector<const sector*>           nears(const fb::model::point16_t& pivot) const;
    std::vector<std::shared_ptr<object>> objects(const fb::model::point16_t& pivot, OBJECT_TYPE type) const;
    std::vector<std::shared_ptr<object>> objects(OBJECT_TYPE type) const;

private:
    uint32_t index(const fb::model::point16_t& position) const;
};

} // namespace fb::game

#endif // !__SECTOR_H__
