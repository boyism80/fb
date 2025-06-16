#ifndef __DOOR_H__
#define __DOOR_H__

#include <fb/lua.h>
#include <fb/model/model.h>

namespace fb::game {

/**
 * @brief      Forward declaration of the map class.
 */
class map;
/**
 * @brief      Forward declaration of the character class.
 */
class character;

/**
 * @brief      Represents an interactive door object in the game world.
 *
 *             This class manages door objects that can be opened, closed, locked, and unlocked
 *             by players. Doors provide access control between different areas of the game world
 *             and can be scripted for complex interactions. They support state management and
 *             integration with the Lua scripting system for dynamic behavior.
 *
 *             Key features:
 *             - Open/close state management
 *             - Lock/unlock functionality for access control
 *             - Position and dimension management on maps
 *             - Lua scripting integration for custom door logic
 *             - Event-driven interaction system
 *             - Persistent state across server restarts
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
     * @param[in]  <unnamed>  The source door object (copy constructor is deleted)
     */
    door(const door&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~door();

public:
    // const door::model&          based() const;
    //
    // @return     Reference to the door's model data
    //
    /**
     * @brief      Toggles the door's open/closed state.
     *
     * @return     True if the door state was successfully toggled, false otherwise
     */
    bool toggle();
    /**
     * @brief      Checks if the door is currently open.
     *
     * @return     True if the door is open, false if closed
     */
    bool opened() const;
    /**
     * @brief      Checks if the door is currently locked.
     *
     * @return     True if the door is locked, false if unlocked
     */
    bool locked() const;
    /**
     * @brief      Sets the lock state of the door.
     *
     * @param[in]  value  True to lock the door, false to unlock it
     */
    void lock(bool value);
};

/**
 * @brief      Lua binding interface for door functionality.
 */
struct door::builtin
{
    /**
     * @brief      Lua binding for toggling door open/closed state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_toggle(lua_State* L);
    /**
     * @brief      Lua binding for checking if door is locked.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_locked(lua_State* L);
    /**
     * @brief      Lua binding for setting door lock state.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_lock(lua_State* L);
    /**
     * @brief      Lua binding for checking if door is opened.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_opened(lua_State* L);
};

} // namespace fb::game

#endif // !__DOOR_H__