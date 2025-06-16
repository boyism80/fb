#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <fb/lua.h>
#include <fb/stream.h>
#include <fb/protocol/header.h>
#include <fb/thread_switchable.h>
#include <fb/model/model.h>
#include <fb/game/spell.h>
#include <random.h>
#include <async/task.h>
#include <shared_mutex>

using namespace fb::model::enum_value;

namespace fb::game {

/**
 * @brief      Forward declaration of the game context class.
 */
class context;
/**
 * @brief      Forward declaration of the map class.
 */
class map;
/**
 * @brief      Forward declaration of the character class.
 */
class character;
/**
 * @brief      Forward declaration of the buffs container class.
 */
class buffs;
/**
 * @brief      Forward declaration of the objects container class.
 */
class objects;
/**
 * @brief      Forward declaration of the items container class.
 */
class items;
/**
 * @brief      Forward declaration of the sector class.
 */
class sector;
/**
 * @brief      Forward declaration of the sectors container class.
 */
class sectors;

/**
 * @brief      Base class for all interactive objects in the game world.
 *
 *             This class serves as the foundation for all objects that can exist in the game world,
 *             including characters, NPCs, items, doors, and other interactive elements. It provides
 *             core functionality for positioning, movement, visibility, communication, and interaction
 *             within the game environment.
 *
 *             Key features:
 *             - Spatial positioning and movement within maps
 *             - Line-of-sight calculations and visibility management
 *             - Inter-object communication and chat system
 *             - Buff/debuff system integration
 *             - Thread-safe operations with context switching
 *             - Lua scripting integration for game logic
 *             - Event-driven architecture with listener pattern
 *             - Network communication capabilities
 */
class object : public fb::thread_switchable
{
public:
    struct listener_t;
    struct builtin;
    struct initial_params;

public:
    LUA_PROTOTYPE

    friend fb::game::buffs;
    friend fb::game::items;

private:
    fb::game::sector* _sector = nullptr;

protected:
    uint32_t                 _sequence = 0;
    const fb::model::object& _model;
    fb::model::point16_t     _position  = fb::model::point16_t(0, 0);
    DIRECTION                _direction = DIRECTION::BOTTOM;
    fb::game::map*           _map       = nullptr;

public:
    listener_t&        listener;
    fb::game::context& context;
    fb::game::buffs    buffs;

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The game context managing this object
     * @param[in]  model    The object model data containing name, appearance, and other settings
     * @param[in]  c        Initial parameters for object construction
     */
    object(fb::game::context& context, const fb::model::object& model, const initial_params& c);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  right  The source object to copy from
     */
    object(const object& right);

public:
    /**
     * @brief      Destroys the object.
     */
    virtual ~object();

private:
    /**
     * @brief      Removes the object from its current map and cleans up references.
     */
    void leave();

    /**
     * @brief      Checks if two positions have line of sight on a map.
     *
     * @param[in]  me    Source position
     * @param[in]  you   Target position
     * @param[in]  map   The map to check line of sight on
     *
     * @return     True if positions have line of sight, false otherwise
     */
    static bool sight(const fb::model::point16_t me, const fb::model::point16_t you, const fb::game::map* map);

public:
    /**
     * @brief      Gets the underlying model cast to a specific type.
     *
     * @tparam     T     The model type to cast to
     *
     * @return     Reference to the model cast as the specified type
     */
    template <typename T> const T& based() const
    {
        return static_cast<const T&>(this->_model);
    }

public:
    [[nodiscard]] virtual async::task<void> destroy(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Sends a data stream to the object.
     *
     * @param[in]  stream   The data stream to send
     * @param[in]  encrypt  Whether to encrypt the data
     * @param[in]  wrap     Whether to wrap the data in protocol headers
     *
     * @return     Number of bytes sent
     */
    virtual async::task<size_t> send(const fb::stream& stream, bool encrypt = true, bool wrap = true);

    /**
     * @brief      Sends a protocol response to the object.
     *
     * @param[in]  response  The protocol response to send
     * @param[in]  encrypt   Whether to encrypt the response
     * @param[in]  wrap      Whether to wrap the response in protocol headers
     *
     * @return     Number of bytes sent
     */
    virtual async::task<size_t> send(const fb::protocol::header& response, bool encrypt = true, bool wrap = true);

    /**
     * @brief      Gets the object's sequence number.
     *
     * @return     The current sequence number
     */
    uint32_t sequence() const;

    /**
     * @brief      Sets the object's sequence number.
     *
     * @param[in]  value  The new sequence number
     */
    void sequence(uint32_t value);

    /**
     * @brief      Gets the underlying model object.
     *
     * @return     Reference to the base model object
     */
    const fb::model::object& based() const;

    /**
     * @brief      Checks if the object is of a specific type.
     *
     * @param[in]  type  The object type to check against
     *
     * @return     True if the object is of the specified type, false otherwise
     */
    bool is(OBJECT_TYPE type) const;

    /**
     * @brief      Gets the object's name.
     *
     * @return     Reference to the object's name string
     */
    virtual const std::string& name() const;

    /**
     * @brief      Gets the object's visual appearance ID.
     *
     * @return     The appearance/sprite ID for rendering
     */
    virtual uint16_t look() const;

    /**
     * @brief      Gets the object's color value.
     *
     * @return     The color value for rendering
     */
    virtual uint8_t color() const;

    /**
     * @brief      Gets the object's type identifier.
     *
     * @return     The object type.
     */
    virtual OBJECT_TYPE what() const;

    /**
     * @brief      Updates the object's external appearance to all nearby objects.
     *
     * @param[in]  light  Whether to use light update (minimal data) or full update
     */
    virtual void update_external(bool light);

    /**
     * @brief      Updates the object's external appearance to a specific target.
     *
     * @param      you    The target object to send the update to
     * @param[in]  light  Whether to use light update (minimal data) or full update
     */
    virtual void update_external(object& you, bool light);

    /**
     * @brief      Checks if the object is in super hide mode (invisible to all).
     *
     * @return     True if the object is super hidden, false otherwise
     */
    virtual bool super_hide() const;

    /**
     * @brief      Checks if this object is hidden from a specific target.
     *
     * @param[in]  target  The target object to check visibility against
     *
     * @return     True if this object is hidden from the target, false otherwise
     */
    virtual bool hidden(const object& target) const;

    /**
     * @brief      Sends a chat message from this object.
     *
     * @param[in]  message    The message text to send
     * @param[in]  chat_type  The type of chat (normal, shout, etc.)
     * @param[in]  decorate   Whether to add decorative formatting to the message
     */
    void chat(const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL, bool decorate = true);

    /**
     * @brief      Gets the object's current position.
     *
     * @return     Reference to the object's position coordinates
     */
    const fb::model::point16_t& position() const;

    /**
     * @brief      Sets the object's position using coordinates.
     *
     * @param[in]  x        The X coordinate
     * @param[in]  y        The Y coordinate
     * @param[in]  refresh  Whether to refresh the object's visual state
     *
     * @return     True if position was set successfully, false otherwise
     */
    virtual bool position(uint16_t x, uint16_t y, bool refresh = false);

    /**
     * @brief      Sets the object's position using a point structure.
     *
     * @param[in]  position  The new position coordinates
     * @param[in]  refresh   Whether to refresh the object's visual state
     *
     * @return     True if position was set successfully, false otherwise
     */
    virtual bool position(const fb::model::point16_t position, bool refresh = false);

    /**
     * @brief      Moves the object in its current direction.
     *
     * @return     True if the move was successful, false otherwise
     */
    bool move();

    /**
     * @brief      Moves the object in a specific direction.
     *
     * @param[in]  direction  The direction to move in
     *
     * @return     True if the move was successful, false otherwise
     */
    virtual bool move(DIRECTION direction);

    /**
     * @brief      Gets the object's X coordinate.
     *
     * @return     The X coordinate value
     */
    uint16_t x() const;

    /**
     * @brief      Sets the object's X coordinate.
     *
     * @param[in]  value  The new X coordinate value
     *
     * @return     True if the coordinate was set successfully, false otherwise
     */
    bool x(uint16_t value);

    /**
     * @brief      Gets the object's Y coordinate.
     *
     * @return     The Y coordinate value
     */
    uint16_t y() const;

    /**
     * @brief      Sets the object's Y coordinate.
     *
     * @param[in]  value  The new Y coordinate value
     *
     * @return     True if the coordinate was set successfully, false otherwise
     */
    bool y(uint16_t value);

    /**
     * @brief      Gets the object's facing direction.
     *
     * @return     The current facing direction
     */
    DIRECTION direction() const;

    /**
     * @brief      Sets the object's facing direction.
     *
     * @param[in]  value  The new facing direction
     *
     * @return     True if the direction was set successfully, false otherwise
     */
    bool direction(DIRECTION value);

    /**
     * @brief      Moves the object to a different map.
     *
     * @param      map           The target map to move to
     * @param[in]  destroy_type  How to handle the object when leaving current map
     *
     * @return     True if the map change was successful, false otherwise
     */
    virtual async::task<bool> map(fb::game::map* map, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Moves the object to a different map at a specific position.
     *
     * @param      map           The target map to move to
     * @param[in]  position      The position on the target map
     * @param[in]  destroy_type  How to handle the object when leaving current map
     *
     * @return     True if the map change was successful, false otherwise
     */
    virtual async::task<bool> map(fb::game::map*              map,
                                  const fb::model::point16_t& position,
                                  DESTROY_TYPE                destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Gets the map that this object is currently on.
     *
     * @return     Pointer to the current map, or nullptr if not on any map
     */
    fb::game::map* map() const;

    /**
     * @brief      Updates the object's sector information on the current map.
     */
    void update_sector();

    /**
     * @brief      Checks if this object can see a specific position.
     *
     * @param[in]  position  The position to check line of sight to
     *
     * @return     True if the position is visible, false if blocked
     */
    bool sight(const fb::model::point16_t& position) const;

    /**
     * @brief      Checks if this object can see another object.
     *
     * @param[in]  object  The target object to check line of sight to
     *
     * @return     True if the target object is visible, false if blocked or hidden
     */
    bool sight(const fb::game::object& object) const;

    /**
     * @brief      Gets the position at the specified side direction from this object.
     *
     * @param[in]  direction  The direction to get the side position for
     * @param[in]  step       The number of steps in that direction
     *
     * @return     The position coordinates at the specified side and distance
     */
    fb::model::point16_t side_position(DIRECTION direction, int step = 1) const;

    /**
     * @brief      Gets the position in front of this object.
     *
     * @param[in]  step  The number of steps forward from current position
     *
     * @return     The position coordinates in front of this object
     */
    fb::model::point16_t front_position(int step = 1) const;

    /**
     * @brief      Gets the first object at the specified side direction.
     *
     * @param[in]  direction  The direction to look for objects
     * @param[in]  type       The type of object to find (UNKNOWN for any type)
     *
     * @return     Pointer to the first object found, or nullptr if none
     */
    object* side(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Gets all objects at the specified side direction.
     *
     * @param[in]  direction  The direction to look for objects
     * @param[in]  type       The type of object to find (UNKNOWN for any type)
     *
     * @return     Vector of pointers to all objects found in that direction
     */
    std::vector<object*> sides(DIRECTION direction, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Gets the first object directly in front of this object.
     *
     * @param[in]  type  The type of object to find (UNKNOWN for any type)
     *
     * @return     Pointer to the first object found in front, or nullptr if none
     */
    object* forward(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Gets all objects directly in front of this object.
     *
     * @param[in]  type  The type of object to find (UNKNOWN for any type)
     *
     * @return     Vector of pointers to all objects found in front
     */
    std::vector<object*> forwards(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Calculates the exact distance to another object.
     *
     * @param[in]  right  The target object to measure distance to
     *
     * @return     The exact distance as a floating-point value
     */
    double distance(const object& right) const;

    /**
     * @brief      Calculates the squared distance to another object (faster than exact distance).
     *
     * @param[in]  right  The target object to measure distance to
     *
     * @return     The squared distance as an integer value
     */
    uint32_t distance_sqrt(const object& right) const;

    /**
     * @brief      Checks if this object meets the specified conditions.
     *
     * @param[in]  conditions  The list of DSL conditions to evaluate
     *
     * @return     True if all conditions are met, false otherwise
     */
    virtual bool condition(const std::vector<fb::model::dsl>& conditions) const;

    /**
     * @brief      Checks if this object is available for interaction.
     *
     * @return     True if the object is available, false if busy or unavailable
     */
    virtual bool available() const;

    /**
     * @brief      Hides this object from all other objects.
     *
     * @param[in]  destroy_type  The type of destruction/hiding to perform
     */
    virtual void hide(DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Hides this object from a specific target object.
     *
     * @param      to            The target object to hide from
     * @param[in]  destroy_type  The type of destruction/hiding to perform
     */
    virtual void hide(fb::game::object& to, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT);

    /**
     * @brief      Gets all objects within line of sight.
     *
     * @param[in]  type  The type of object to find (UNKNOWN for any type)
     *
     * @return     Vector of pointers to all visible objects
     */
    std::vector<object*> sight_in(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Gets all nearby objects within the same sector.
     *
     * @param[in]  type                 The type of object to find (UNKNOWN for any type)
     * @param[in]  contains_super_hide  Whether to include super hidden objects
     *
     * @return     Vector of pointers to all nearby objects
     */
    std::vector<object*> nears(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN, bool contains_super_hide = false) const;

    /**
     * @brief      Gets the thread that this object belongs to.
     *
     * @return     Pointer to the thread managing this object
     */
    fb::thread* thread() const override;

    /**
     * @brief      Updates the object's unique identifier.
     */
    virtual void update_id();

    /**
     * @brief      Updates the object with new map information.
     *
     * @param[in]  map   The map to update with
     */
    virtual void update_map(const fb::game::map& map);

    /**
     * @brief      Updates the background music for this object.
     *
     * @param[in]  bgm     The background music ID to play
     * @param[in]  volume  The volume level (0-255)
     */
    virtual void update_bgm(uint16_t bgm, uint8_t volume);

    /**
     * @brief      Updates the object's position information to clients.
     */
    virtual void update_position();

    /**
     * @brief      Plays a sound effect for this object.
     *
     * @param[in]  sound  The sound effect to play
     */
    void sound(SOUND sound);

    /**
     * @brief      Applies a visual effect to this object.
     *
     * @param[in]  value  The effect ID to apply
     */
    void effect(uint8_t value);

public:
    /**
     * @brief      Equality operator.
     *
     * @param[in]  right  The source object to compare with
     *
     * @return     The result of the equality
     */
    bool operator== (const object& right) const;

    /**
     * @brief      Inequality operator.
     *
     * @param[in]  right  The source object to compare with
     *
     * @return     The result of the inequality
     */
    bool operator!= (const object& right) const;
};

/**
 * @brief      Event listener interface for object-related events.
 *
 *             This interface defines all the callback methods that handle various
 *             object events such as creation, destruction, movement, communication,
 *             and visual updates. Implementations of this interface receive
 *             notifications when objects perform actions or change state.
 */
struct object::listener_t
{
    /**
     * @brief      Called when an object sends a chat message.
     *
     * @param      me         The object that sent the chat message
     * @param[in]  message    The chat message text
     * @param[in]  chat_type  The type of chat (normal, shout, whisper, etc.)
     */
    virtual void on_chat(fb::game::object& me, const std::string& message, CHAT_TYPE chat_type = CHAT_TYPE::NORMAL) = 0;

    /**
     * @brief      Called when an object changes its facing direction.
     *
     * @param      me    The object that changed its facing direction
     */
    virtual void on_direction(fb::game::object& me) = 0;

    /**
     * @brief      Called when an object's external appearance is updated to all nearby objects.
     *
     * @param      me     The object whose appearance is being updated
     * @param[in]  light  Whether this is a light update (minimal data) or full update
     */
    virtual void on_update_external(fb::game::object& me, bool light) = 0;

    /**
     * @brief      Called when an object's external appearance is updated to a specific target object.
     *
     * @param      me     The object whose appearance is being updated
     * @param      you    The target object receiving the appearance update
     * @param[in]  light  Whether this is a light update (minimal data) or full update
     */
    virtual void on_update_external(fb::game::object& me, fb::game::object& you, bool light) = 0;

    /**
     * @brief      Called when an object is hidden from all other objects.
     *
     * @param      me            The object being hidden
     * @param[in]  destroy_type  The type of hiding/destruction being performed
     */
    virtual void on_hide(fb::game::object& me, DESTROY_TYPE destroy_type = DESTROY_TYPE::DEFAULT) = 0;

    /**
     * @brief      Called when an object is hidden from a specific target object.
     *
     * @param      me            The object being hidden
     * @param      you           The target object that can no longer see the hidden object
     * @param[in]  destroy_type  The type of hiding/destruction being performed
     */
    virtual void on_hide(fb::game::object& me, fb::game::object& you, DESTROY_TYPE destroy_type) = 0;

    /**
     * @brief      Called when an object moves to a new position.
     *
     * @param      me      The object that moved
     * @param[in]  before  The object's previous position before the move
     */
    virtual void on_move(fb::game::object& me, const fb::model::point16_t& before) = 0;

    /**
     * @brief      Called when a buff is applied to an object.
     *
     * @param      me    The object receiving the buff
     * @param      buff  The buff being applied
     */
    virtual void on_buff(fb::game::object& me, fb::game::buff& buff) = 0;

    /**
     * @brief      Called when a buff is removed from an object.
     *
     * @param      me    The object losing the buff
     * @param      buff  The buff being removed
     */
    virtual void on_unbuff(fb::game::object& me, fb::game::buff& buff) = 0;

    /**
     * @brief      Called when an object is created and added to the game world.
     *
     * @param      me    The object being created
     */
    virtual void on_create(fb::game::object& me) = 0;

    /**
     * @brief      Called when an object is destroyed and removed from the game world.
     *
     * @param      me    The object being destroyed
     */
    virtual void on_destroy(fb::game::object& me) = 0;

    /**
     * @brief      Called when an object plays a sound effect.
     *
     * @param      ch     The object playing the sound effect
     * @param[in]  sound  The sound effect being played
     */
    virtual void on_sound(fb::game::object& ch, SOUND sound) = 0;

    /**
     * @brief      Called when an object applies a visual effect.
     *
     * @param      ch     The object applying the visual effect
     * @param[in]  value  The effect ID being applied
     */
    virtual void on_effect(fb::game::object& ch, uint8_t value) = 0;
};

/**
 * @brief      Lua bindings for object methods.
 */
struct object::builtin
{
public:
    /**
     * @brief      Lua binding to get the object's model data.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding to get the object's unique ID.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_id(lua_State* L);

    /**
     * @brief      Lua binding for object equality comparison.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_eq(lua_State* L);

    /**
     * @brief      Lua binding to convert object to string representation.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_tostring(lua_State* L);

    /**
     * @brief      Lua binding to get the object's name.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_name(lua_State* L);

    /**
     * @brief      Lua binding to play a sound effect for the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_sound(lua_State* L);

    /**
     * @brief      Lua binding to get or set the object's position.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_position(lua_State* L);

    /**
     * @brief      Lua binding to get the position in front of the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_front_position(lua_State* L);

    /**
     * @brief      Lua binding to get or set the object's facing direction.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_direction(lua_State* L);

    /**
     * @brief      Lua binding to send a chat message from the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_chat(lua_State* L);

    /**
     * @brief      Lua binding to apply a buff to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buff(lua_State* L);

    /**
     * @brief      Lua binding to remove a buff from the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_unbuff(lua_State* L);

    /**
     * @brief      Lua binding to check if the object has a specific buff.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_isbuff(lua_State* L);

    /**
     * @brief      Lua binding to apply a visual effect to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_effect(lua_State* L);

    /**
     * @brief      Lua binding to get the object's current map.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_map(lua_State* L);

    /**
     * @brief      Lua binding to create an item for the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_mkitem(lua_State* L);

    /**
     * @brief      Lua binding to get objects within line of sight.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_sight_in(lua_State* L);

    /**
     * @brief      Lua binding to get nearby objects.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding to get objects in front of this object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_front(lua_State* L);

    /**
     * @brief      Lua binding to check if the object is of a specific type.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_is(lua_State* L);

    /**
     * @brief      Lua binding to get the object's thread.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_thread(lua_State* L);

    /**
     * @brief      Lua binding to get the object's pointer address.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_ptr(lua_State* L);

    /**
     * @brief      Lua binding to get the nearest object of a specific type.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_near(lua_State* L);

    /**
     * @brief      Lua binding to get all buffs applied to the object.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_buffs(lua_State* L);

    /**
     * @brief      Lua binding to check if the object is hidden.
     *
     * @param      L     The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_hidden(lua_State* L);
};

/**
 * @brief      Initialization parameters for creating game objects.
 *
 *             This structure contains the essential parameters needed to initialize
 *             a game object, including its position, direction, map assignment,
 *             and unique identifier. These parameters are used during object
 *             construction to set up the initial state.
 */
struct object::initial_params
{
public:
    uint32_t                   id; ///< Unique object identifier (0xFFFFFFFF for auto-assignment)
    const fb::model::point16_t position  = fb::model::point16_t(); ///< Initial position coordinates on the map
    DIRECTION                  direction = DIRECTION::BOTTOM;      ///< Initial facing direction
    fb::game::map*             map       = nullptr; ///< Pointer to the map where the object will be placed
};

} // namespace fb::game

#endif // !__OBJECT_H__
