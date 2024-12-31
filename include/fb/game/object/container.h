#ifndef __OBJECT_CONTAINER_H__
#define __OBJECT_CONTAINER_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <memory>
#include <fb/game/door.h>
#include <fb/game/sector.h>
#include <fb/stream.h>
#include <fb/config.h>
#include <fb/container.h>

namespace fb::game {

class object;
class map;
class mob;
class character;
class context;

/**
 * @brief      This class describes object_container.
 */
class object_container
{
public:
    using ptrs                  = std::unordered_map<uint32_t, std::unique_ptr<object>>;
    using refs                  = std::unordered_map<uint32_t, object&>;
    using iterator              = refs::iterator;
    using const_iterator        = refs::const_iterator;
    using handle_predicate_type = std::function<bool(object&)>;

private:
    ptrs     _ptrs;
    refs     _refs;
    uint32_t _sequence = 1;

public:
    fb::game::map& owner;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      map   The map
     */
    object_container(fb::game::map& map);
    /**
     * @brief      Destroys the object.
     */
    ~object_container() = default;

private:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t empty_seq();

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    iterator begin();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    iterator end();
    /**
     * @brief      { function_description }
     *
     * @return     The constant iterator.
     */
    const_iterator begin() const;
    /**
     * @brief      { function_description }
     *
     * @return     The constant iterator.
     */
    const_iterator end() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint32_t size() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  i     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    object& at(uint32_t i);
    /**
     * @brief      { function_description }
     *
     * @param      obj   The object
     */
    void push(object& obj);
    /**
     * @brief      Pops the given fd.
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    object& pop(uint32_t fd);
    /**
     * @brief      Pops the given object.
     *
     * @param      obj   The object
     *
     * @return     { description_of_the_return_value }
     */
    object& pop(object& obj);
    /**
     * @brief      { function_description }
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    object* try_pop(uint32_t fd);
    /**
     * @brief      { function_description }
     *
     * @param      obj   The object
     *
     * @return     { description_of_the_return_value }
     */
    object* try_pop(object& obj);
    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     * @param[in]  fn    The function
     */
    void foreach (OBJECT_TYPE type, const handle_predicate_type& fn);
    /**
     * @brief      { function_description }
     *
     * @param[in]  obj   The object
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(const object& obj) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool contains(uint32_t fd) const;

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  fd    { parameter_description }
     *
     * @return     The result of the array indexer
     */
    object* operator[] (uint32_t fd);
};

} // namespace fb::game

#endif