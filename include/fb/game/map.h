#ifndef __MAP_H__
#define __MAP_H__

#include <fb/game/door/container.h>
#include <fb/game/object/container.h>
#include <fb/game/sector.h>

namespace fb::game {

/**
 * @brief      This class describes a map of .
 */
class map : public lua::luable, public fb::thread_switchable
{
public:
    LUA_PROTOTYPE

public:
    static constexpr uint32_t MAX_SCREEN_WIDTH   = 17;
    static constexpr uint32_t HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static constexpr uint32_t MAX_SCREEN_HEIGHT  = 15;
    static constexpr uint32_t HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
    /**
     * @brief      { struct_description }
     */
    struct tile;

public:
    using unique_tiles  = std::unique_ptr<tile[]>;
    using unique_sector = std::unique_ptr<sectors>;

private:
    size16_t      _size  = size16_t(0, 0);
    unique_tiles  _tiles = nullptr;
    unique_sector _sectors;

public:
    const fb::game::context& context;
    const fb::model::map&    model;
    object_container         objects = object_container(*this);
    door_container           doors;
    const bool               active;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context
     * @param[in]  model    The model
     * @param[in]  active   The active
     * @param[in]  data     The data
     * @param[in]  size     The size
     */
    map(fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    map(const fb::game::map&) = delete;
    /**
     * @brief      Destroys the object.
     */
    ~map();

private:
    /**
     * @brief      Loads a door.
     */
    void update_door();

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  p     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint64_t index(const point16_t& p) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  i     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    point16_t point(uint64_t i) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool blocked(uint16_t x, uint16_t y) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  x       { parameter_description }
     * @param[in]  y       { parameter_description }
     * @param[in]  option  The option
     *
     * @return     { description_of_the_return_value }
     */
    bool block(uint16_t x, uint16_t y, bool option);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t width() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint16_t height() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    size16_t size() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool loaded() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    bool existable(const point16_t position) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    bool movable(const point16_t position) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  object     The object
     * @param[in]  direction  The direction
     *
     * @return     { description_of_the_return_value }
     */
    bool movable(const object& object, DIRECTION direction) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     * @param[in]  step    The step
     *
     * @return     { description_of_the_return_value }
     */
    bool movable_forward(const object& object, uint16_t step = 1) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    const fb::model::warp* warpable(const point16_t& position) const;
    /**
     * @brief      Updates the given object.
     *
     * @param      object  The object
     *
     * @return     { description_of_the_return_value }
     */
    bool update(object& object);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool activated() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  pivot  The pivot
     * @param[in]  type   The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> nears(const point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  pivot  The pivot
     * @param[in]  type   The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> belows(const point16_t& pivot, OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  type  The type
     *
     * @return     { description_of_the_return_value }
     */
    std::vector<object*> activateds(OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN);

    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    fb::thread* thread() const override;

public:
    /**
     * @brief      Called on timer.
     *
     * @param[in]  elapsed_milliseconds  The elapsed milliseconds
     */
    [[nodiscard]] async::task<void> on_timer(uint64_t elapsed_milliseconds);

public:
    /**
     * @brief      Function call operator.
     *
     * @param[in]  x     { parameter_description }
     * @param[in]  y     { parameter_description }
     *
     * @return     The result of the function call
     */
    tile* operator() (uint16_t x, uint16_t y) const;

public:
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_width(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_height(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_area(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_objects(lua_State* lua);

    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_nears(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_movable(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_door(lua_State* lua);
    /**
     * @brief      { function_description }
     *
     * @param      lua   The lua
     *
     * @return     { description_of_the_return_value }
     */
    static int builtin_doors(lua_State* lua);
    /**
     * @brief      Determines if builtin contains.
     *
     * @param      lua   The lua
     *
     * @return     True if builtin contains, False otherwise.
     */
    static int builtin_contains(lua_State* lua);
};

/**
 * @brief      { struct_description }
 */
struct map::tile
{
    uint16_t id;
    uint16_t object;
    bool     blocked;
};

} // namespace fb::game

#endif // !__MAP_H__
