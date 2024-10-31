#ifndef __DOOR_H__
#define __DOOR_H__

#include <vector>
#include <mmo.h>
#include <lua.h>
#include <model.h>

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
 * @brief      This class describes a door.
 */
class door : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    /**
     * @brief      This class describes a model.
     */
    class model;

private:
    bool _opened = false;
    bool _locked = false;

public:
    const fb::game::map&   map;
    const fb::model::door& model;
    const point16_t        position;
    const uint16_t         width;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map       The map
     * @param[in]  model     The model
     * @param[in]  position  The position
     * @param[in]  opened    Indicates if opened
     */
    door(const fb::game::map& map, const fb::model::door& model, const point16_t& position, bool opened);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    door(const door&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~door();

public:
    // const door::model&          based() const;
    //
    // @return     { description_of_the_return_value }
    //
    bool toggle();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool opened() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool locked() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  value  The value
     */
    void lock(bool value);

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_toggle(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_locked(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_lock(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_opened(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_update(lua_State* lua);
};

/**
 * @brief      This class describes doors.
 */
class doors : private std::unordered_map<uint64_t, std::unique_ptr<door>>
{
public:
    /**
     * @brief      This class describes an iterator.
     */
    class iterator;
    /**
     * @brief      This class describes a constant iterator.
     */
    class const_iterator;

    friend class iterator;
    friend class const_iterator;

public:
    const fb::game::map& map;

public:
    using unordered_map<uint64_t, std::unique_ptr<door>>::begin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::end;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cbegin;
    using unordered_map<uint64_t, std::unique_ptr<door>>::cend;
    using unordered_map<uint64_t, std::unique_ptr<door>>::size;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map   The map
     */
    doors(const fb::game::map& map);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    doors(const doors&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~doors();

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
     * @param[in]  position  The position
     * @param[in]  model     The model
     * @param[in]  opened    Indicates if opened
     */
    void add(const point16_t& position, const fb::model::door& model, bool opened);
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    door* find(const point16_t position) const;
    /**
     * @brief      Searches for the first match.
     *
     * @param[in]  session  The session
     *
     * @return     { description_of_the_return_value }
     */
    door* find(const character& session) const;
};

/**
 * @brief      This class describes an iterator.
 */
class doors::iterator : public std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator
{
public:
    std::optional<std::pair<point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  i          { parameter_description }
     * @param[in]  container  The container
     */
    iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::iterator& i, const doors& container);
    /**
     * @brief      Destroys the object.
     */
    ~iterator() = default;

public:
    /**
     * @brief      Multiplication operator.
     *
     * @return     The result of the multiplication
     */
    std::pair<point16_t, door&> operator* ();
};

/**
 * @brief      This class describes a constant iterator.
 */
class doors::const_iterator : public std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator
{
public:
    const std::optional<std::pair<point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  i          { parameter_description }
     * @param[in]  container  The container
     */
    const_iterator(const std::unordered_map<uint64_t, std::unique_ptr<door>>::const_iterator& i,
                   const doors&                                                               container);
    /**
     * @brief      Destroys the object.
     */
    ~const_iterator() = default;

public:
    /**
     * @brief      Multiplication operator.
     *
     * @return     The result of the multiplication
     */
    const std::pair<point16_t, door&> operator* () const;
};

}} // namespace fb::game

#endif // !__DOOR_H__