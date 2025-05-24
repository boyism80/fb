#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <fb/game/object.h>
#include <set>

namespace fb::game {

/**
 * @brief      This class describes a map of .
 */
class map;
/**
 * @brief      This class describes a character.
 */
class character;
/**
 * @brief      This class describes sectors.
 */
class sectors;

/**
 * @brief      This class describes a sector.
 */
class sector : private std::vector<fb::game::object*>
{
private:
    using super = std::vector<fb::game::object*>;

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
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id    The identifier
     */
    sector(uint32_t id);
    /**
     * @brief      Destroys the object.
     */
    ~sector();

public:
    /**
     * @brief      { function_description }
     *
     * @param      object  The object
     */
    void push(fb::game::object& object);
    /**
     * @brief      { function_description }
     *
     * @param      object  The object
     */
    void erase(fb::game::object& object);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t id() const;
    /**
     * @brief      Determines if active.
     *
     * @return     True if active, False otherwise.
     */
    bool is_active() const;
};

/**
 * @brief      This class describes sectors.
 */
class sectors
{
public:
    using unique_sectors = std::vector<std::unique_ptr<sector>>;

private:
    const fb::model::size16_t _map_size = fb::model::size16_t(0, 0);
    const fb::model::size16_t _size     = fb::model::size16_t(0, 0);
    const uint32_t            _rows = 0, _columns = 0;
    const uint32_t            _count = 0;
    unique_sectors            _pool;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map_size  The map size
     * @param[in]  size      The size
     */
    sectors(const fb::model::size16_t& map_size, const fb::model::size16_t& size);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    sectors(const sectors&) = delete;
    /**
     * @brief      Constructs a new instance.
     *
     * @param      <unnamed>  { parameter_description }
     */
    sectors(sectors&&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~sectors() = default;

public:
    /**
     * @brief      Assignment operator.
     *
     * @param      <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    sectors& operator= (sectors&) = delete;
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  <unnamed>  { parameter_description }
     *
     * @return     The result of the assignment
     */
    sectors& operator= (const sectors&) = delete;

private:
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t index(const fb::model::point16_t& position) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::set<sector*> active_sectors() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    sector* at(const fb::model::point16_t& position) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    sector* at(uint32_t index) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<sector*> nears(uint32_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  pivot  The pivot
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<sector*> nears(const fb::model::point16_t& pivot) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  pivot  The pivot
     * @param[in]  type   The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> objects(const fb::model::point16_t& pivot, OBJECT_TYPE type) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> objects(OBJECT_TYPE type) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool is_active() const;
};

} // namespace fb::game

#endif // !__SECTOR_H__
