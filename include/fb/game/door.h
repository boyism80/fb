#ifndef __DOOR_H__
#define __DOOR_H__

#include <fb/lua.h>
#include <fb/model/model.h>

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
 * @brief      This class describes a door.
 */
class door : public lua::luable
{
public:
    LUA_PROTOTYPE

public:
    class model;
    struct builtin;

private:
    bool _opened = false;
    bool _locked = false;

public:
    const fb::game::map&       map;
    const fb::model::door&     model;
    const fb::model::point16_t pivot;
    const uint16_t             width;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  map       The map
     * @param[in]  model     The model
     * @param[in]  opened    Indicates if opened
     */
    door(const fb::game::map& map, const fb::model::door& model, const fb::model::point16_t& pivot, bool opened);
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
};

/**
 * @brief      { struct_description }
 */
struct door::builtin
{
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_toggle(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_locked(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_lock(lua_State* L);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_opened(lua_State* L);
};

} // namespace fb::game

#endif // !__DOOR_H__