#ifndef __MAP_H__
#define __MAP_H__

/**
 * @file    map.h
 * @brief   Game world map system with spatial management and object tracking
 * @author  FB Development Team
 *
 * @details This file implements the map system that represents individual game world
 *          areas in the FB 2D MMORPG. Maps provide spatial organization, collision
 *          detection, object management, and serve as the primary containers for
 *          game world interactions and gameplay.
 *
 *          Key features:
 *          - Tile-based spatial organization with collision detection system
 *          - Sector-based optimization for efficient spatial queries and object management
 *          - Comprehensive object container management for all map entities
 *          - Door and warp point management for map transitions
 *          - Movement validation and pathfinding support
 *          - Thread-safe operations with automatic context switching
 *          - Lua scripting integration for dynamic map behavior and events
 *          - Real-time object tracking and proximity-based queries
 *          - Map loading and initialization from binary data files
 *          - Active/inactive state management for performance optimization
 *
 * @note    Maps are the fundamental spatial containers where all game interactions
 *          occur, providing the foundation for movement, combat, and social activities.
 */

#include <fb/game/door/container.h>
#include <fb/game/object/container.h>
#include <fb/game/sector.h>

namespace fb::game {

/**
 * @brief      Represents a game world map with spatial management and object tracking.
 *
 *             This class manages a single map in the game world, providing spatial organization,
 *             object tracking, movement validation, and sector-based optimization. It handles
 *             all aspects of map functionality including tile management, object placement,
 *             collision detection, and warp point management.
 *
 *             Key features:
 *             - Tile-based spatial organization with collision detection
 *             - Sector-based optimization for efficient object queries
 *             - Object container management for all map entities
 *             - Door and warp point management
 *             - Thread-safe operations with context switching
 *             - Lua scripting integration for dynamic map behavior
 *             - Movement validation and pathfinding support
 */
class map : public fb::thread_switchable
{
public:
    LUA_PROTOTYPE

public:
    static constexpr uint32_t MAX_SCREEN_WIDTH   = 17;
    static constexpr uint32_t HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
    static constexpr uint32_t MAX_SCREEN_HEIGHT  = 15;
    static constexpr uint32_t HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
    struct tile;
    struct builtin;

public:
    using unique_tiles  = std::unique_ptr<tile[]>;
    using shared_sector = std::shared_ptr<sectors>;

private:
    fb::model::size16_t _size  = fb::model::size16_t(0, 0);
    unique_tiles        _tiles = nullptr;
    shared_sector       _sectors;

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
     * @param[in]  context  The game context that manages this map
     * @param[in]  model    The map model data containing configuration and metadata
     * @param[in]  active   Whether the map should be active and process game logic
     * @param[in]  data     The raw map tile data to load
     * @param[in]  size     The size of the map data in bytes
     */
    map(fb::game::context& context, const fb::model::map& model, bool active, const void* data, size_t size);

    /**
     * @brief      Constructs a new instance.
     *
     * @param[in]  <unnamed>  The source map object (copy constructor is deleted)
     */
    map(const fb::game::map&) = delete;

    /**
     * @brief      Destroys the object.
     */
    ~map();

private:
    /**
     * @brief      Updates and synchronizes all door states on the map.
     */
    void update_door();

public:
    /**
     * @brief      Converts a 2D point to a linear array index for tile access.
     *
     * @param[in]  p     The 2D point coordinates to convert
     *
     * @return     The linear index corresponding to the point position
     */
    uint64_t index(const fb::model::point16_t& p) const;

    /**
     * @brief      Converts a linear array index back to a 2D point coordinate.
     *
     * @param[in]  i     The linear index to convert
     *
     * @return     The 2D point coordinates corresponding to the index
     */
    fb::model::point16_t point(uint64_t i) const;

    /**
     * @brief      Checks if a tile at the specified coordinates is blocked for movement.
     *
     * @param[in]  x     The X coordinate to check
     * @param[in]  y     The Y coordinate to check
     *
     * @return     True if the tile is blocked, false if passable
     */
    bool blocked(uint16_t x, uint16_t y) const;

    /**
     * @brief      Sets or clears the blocked state of a tile at the specified coordinates.
     *
     * @param[in]  x       The X coordinate of the tile to modify
     * @param[in]  y       The Y coordinate of the tile to modify
     * @param[in]  option  True to block the tile, false to unblock it
     *
     * @return     True if the operation was successful, false otherwise
     */
    bool block(uint16_t x, uint16_t y, bool option);

    /**
     * @brief      Gets the width of the map in tiles.
     *
     * @return     The map width in tile units
     */
    uint16_t width() const;

    /**
     * @brief      Gets the height of the map in tiles.
     *
     * @return     The map height in tile units
     */
    uint16_t height() const;

    /**
     * @brief      Gets the dimensions of the map as a size structure.
     *
     * @return     The map size containing width and height
     */
    fb::model::size16_t size() const;

    /**
     * @brief      Checks if the map has been fully loaded and initialized.
     *
     * @return     True if the map is loaded and ready for use, false otherwise
     */
    bool loaded() const;

    /**
     * @brief      Checks if a position is within the valid map boundaries.
     *
     * @param[in]  position  The position coordinates to validate
     *
     * @return     True if the position is within map bounds, false otherwise
     */
    bool in_ground(const fb::model::point16_t position) const;

    /**
     * @brief      Checks if an object can move to a specific position on the map.
     *
     * @param[in]  position  The target position to check for movement validity
     *
     * @return     True if the object can move to the position, false if blocked
     */
    bool movable(const object& object, const fb::model::point16_t position) const;

    /**
     * @brief      Checks if an object can move in a specific direction from its current position.
     *
     * @param[in]  object     The object attempting to move
     * @param[in]  direction  The direction of movement to validate
     *
     * @return     True if movement in the direction is valid, false if blocked
     */
    bool movable(const object& object, DIRECTION direction) const;

    /**
     * @brief      Checks if an object can move forward a specified number of steps.
     *
     * @param[in]  object  The object attempting to move forward
     * @param[in]  step    The number of steps to move forward (default: 1)
     *
     * @return     True if forward movement is possible, false if blocked
     */
    bool movable_forward(const object& object, uint16_t step = 1) const;

    /**
     * @brief      Checks if a position contains a warp point and returns warp information.
     *
     * @param[in]  position  The position to check for warp points
     *
     * @return     Pointer to warp data if position is warpable, nullptr otherwise
     */
    const fb::model::warp* warpable(const fb::model::point16_t& position) const;

    /**
     * @brief      Checks if the map is currently active and processing game logic.
     *
     * @return     True if the map is active, false if inactive or suspended
     */
    bool is_active() const;

    /**
     * @brief      Gets the sector containing the specified position for spatial optimization.
     *
     * @param[in]  position  The position to find the containing sector for
     *
     * @return     Pointer to the sector containing the position, or nullptr if invalid
     */
    std::shared_ptr<fb::game::sector> sector_at(const fb::model::point16_t& position);

    /**
     * @brief      Finds all objects near a pivot point within interaction range.
     *
     * @param[in]  pivot  The center point to search around
     * @param[in]  type   The type of objects to filter for (default: all types)
     *
     * @return     Vector of object pointers within range of the pivot point
     */
    std::vector<std::shared_ptr<fb::game::object>> nears(const fb::model::point16_t& pivot,
                                                         OBJECT_TYPE                 type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Finds all objects at or below a specific position on the map.
     *
     * @param[in]  pivot  The position to search at and below
     * @param[in]  type   The type of objects to filter for (default: all types)
     *
     * @return     Vector of object pointers at or below the specified position
     */
    std::vector<std::shared_ptr<fb::game::object>> belows(const fb::model::point16_t& pivot,
                                                          OBJECT_TYPE type = OBJECT_TYPE::UNKNOWN) const;

    /**
     * @brief      Gets the thread that manages this map's execution context.
     *
     * @return     Pointer to the thread managing this map
     */
    fb::thread* thread() const override;

public:
    /**
     * @brief      Function call operator.
     *
     * @param[in]  x     The X coordinate of the tile to access
     * @param[in]  y     The Y coordinate of the tile to access
     *
     * @return     The result of the function call
     */
    tile* operator() (uint16_t x, uint16_t y) const;
};

struct map::builtin
{
    /**
     * @brief      Lua binding for getting the map's model data.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_model(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's width.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_width(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's height.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_height(lua_State* L);

    /**
     * @brief      Lua binding for getting the map's total area.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_area(lua_State* L);

    /**
     * @brief      Lua binding for getting all objects on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_objects(lua_State* L);

    /**
     * @brief      Lua binding for finding objects near a specific position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_nears(lua_State* L);

    /**
     * @brief      Lua binding for checking if movement is possible.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_movable(lua_State* L);

    /**
     * @brief      Lua binding for getting a specific door on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_door(lua_State* L);

    /**
     * @brief      Lua binding for getting all doors on the map.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_doors(lua_State* L);

    /**
     * @brief      Lua binding for checking if map contains specified object.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_contains(lua_State* L);

    /**
     * @brief      Lua binding for finding objects at or below a position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_belows(lua_State* L);

    /**
     * @brief      Lua binding for getting tile information at coordinates.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_tile(lua_State* L);

    /**
     * @brief      Lua binding for getting objects at a specific position.
     *
     * @param[in]  L  The Lua state
     *
     * @return     Number of return values pushed to Lua stack
     */
    static int builtin_at(lua_State* L);
};

/**
 * @brief      Represents a single tile on the map with visual and collision data.
 */
struct map::tile
{
    uint16_t id;
    uint16_t object;
    bool     blocked;
};

} // namespace fb::game

#endif // !__MAP_H__
