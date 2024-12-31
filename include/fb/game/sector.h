#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <fb/game/object.h>
#include <set>

using namespace fb::model;

namespace fb { namespace game {

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
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  id             The identifier
     * @param[in]  state_changed  The state changed
     */
    sector(uint32_t id, const std::function<void(sector&)>& state_changed);
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
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool activated() const;
};

/**
 * @brief      This class describes sectors.
 */
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
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map_size  The map size
     * @param[in]  size      The size
     */
    sectors(const size16_t& map_size, const size16_t& size);
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
    uint32_t index(const point16_t& position) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    std::set<sector*> activated_sectors() const;

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    sector* at(const point16_t& position) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    sector* at(uint32_t index) const;
    /**
     * @brief      { function_description }
     *
     * @param      object  The object
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t push(fb::game::object& object);

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
    std::vector<sector*> nears(const point16_t& pivot) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  pivot  The pivot
     * @param[in]  type   The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> objects(const point16_t& pivot, OBJECT_TYPE type) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> activated_objects(OBJECT_TYPE type) const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool activated() const;
};

}} // namespace fb::game

#endif // !__SECTOR_H__
