#ifndef __MAP_H__
#define __MAP_H__

#include <stdexcept>
#include <door.h>
#include <sector.h>
#include <fb/container.h>

namespace fb { namespace game {

/**
 * @brief      This class describes an object.
 */
class object;
/**
 * @brief      This class describes a map of .
 */
class map;
/**
 * @brief      This class describes a mob.
 */
class mob;
/**
 * @brief      This class describes a character.
 */
class character;
/**
 * @brief      This class describes a context.
 */
class context;

/**
 * @brief      This class describes objects.
 */
class objects
{
public:
    using ptrs           = std::unordered_map<uint32_t, std::unique_ptr<object>>;
    using refs           = std::unordered_map<uint32_t, object&>;
    using iterator       = refs::iterator;
    using const_iterator = refs::const_iterator;
    using filter_func    = std::function<bool(object&)>;

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
    objects(fb::game::map& map);
    /**
     * @brief      Destroys the object.
     */
    ~objects() = default;

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
    void foreach (OBJECT_TYPE type, const filter_func& fn);
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

/**
 * @brief      This class describes a map of .
 */
class map : public lua::luable
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
    using unique_sector = std::unique_ptr<fb::game::sectors>;

private:
    size16_t      _size  = size16_t(0, 0);
    unique_tiles  _tiles = nullptr;
    unique_sector _sectors;

public:
    const fb::game::context& context;
    const fb::model::map&    model;
    fb::game::objects        objects = fb::game::objects(*this);
    fb::game::doors          doors;
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
    map(const fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size);
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  { parameter_description }
     */
    map(const map&) = delete;
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
    bool movable(const fb::game::object& object, DIRECTION direction) const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  object  The object
     * @param[in]  step    The step
     *
     * @return     { description_of_the_return_value }
     */
    bool movable_forward(const fb::game::object& object, uint16_t step = 1) const;
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
    bool update(fb::game::object& object);
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

public:
    /**
     * @brief      Called on timer.
     *
     * @param[in]  elapsed_milliseconds  The elapsed milliseconds
     */
    void on_timer(uint64_t elapsed_milliseconds);

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

/**
 * @brief      This class describes maps.
 */
class maps : public fb::kv_container<uint32_t, fb::game::map>
{
private:
    std::mutex _mutex;

public:
    const fb::game::context& context;
    const uint32_t           host;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  context  The context
     * @param[in]  host     The host
     */
    maps(const fb::game::context& context, uint32_t host);
    /**
     * @brief      Destroys the object.
     */
    ~maps();

private:
    /**
     * @brief      Loads a data.
     *
     * @param[in]  id      The identifier
     * @param      buffer  The buffer
     *
     * @return     { description_of_the_return_value }
     */
    static bool load_data(uint32_t id, std::vector<char>& buffer);
    /**
     * @brief      Loads a block.
     *
     * @param[in]  id      The identifier
     * @param      buffer  The buffer
     *
     * @return     { description_of_the_return_value }
     */
    static bool load_block(uint32_t id, Json::Value& buffer);

public:
    /**
     * @brief      { function_description }
     *
     * @param[in]  moel  The moel
     */
    void load(const fb::model::map& moel);
    /**
     * @brief      { function_description }
     *
     * @param[in]  name  The name
     *
     * @return     { description_of_the_return_value }
     */
    fb::game::map* name2map(const std::string& name) const;
};

}} // namespace fb::game

#endif // !__MAP_H__
