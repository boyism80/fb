#ifndef __SECTOR_H__
#define __SECTOR_H__

#include <fb/game/object.h>
#include <set>

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
 * @brief      Forward declaration of the sectors container class.
 */
class sectors;

/**
 * @brief      Represents a spatial sector for efficient object management within a map.
 *
 *             This class manages a spatial subdivision of a map, containing all objects
 *             within a specific rectangular area. Sectors are used to optimize object
 *             queries, collision detection, and visibility calculations by reducing the
 *             search space for spatial operations. Each sector tracks its objects and
 *             maintains activation state based on player presence.
 *
 *             Key features:
 *             - Efficient spatial object storage and retrieval
 *             - Character count tracking for activation management
 *             - Dynamic activation/deactivation based on player presence
 *             - STL container interface for object iteration
 *             - Thread-safe object management
 */
class sector : private std::vector<std::shared_ptr<fb::game::object>>
{
private:
    using super = std::vector<std::shared_ptr<fb::game::object>>;

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
     * @brief      Constructs a new sector with the specified identifier.
     *
     *             Initializes an empty sector with the given ID, ready to contain
     *             objects within its spatial boundaries.
     *
     * @param[in]  id    The unique identifier for this sector.
     */
    sector(uint32_t id);
    /**
     * @brief      Destroys the sector and cleans up resources.
     */
    ~sector();

public:
    /**
     * @brief      Adds an object to this sector.
     *
     *             Registers the specified object as being located within this sector's
     *             spatial boundaries. Updates character count if the object is a character.
     *
     * @param      object  The object to add to this sector.
     */
    void push(std::shared_ptr<fb::game::object> object);
    /**
     * @brief      Removes an object from this sector.
     *
     *             Unregisters the specified object from this sector. Updates character
     *             count and activation state if necessary.
     *
     * @param      object  The object to remove from this sector.
     */
    void erase(std::shared_ptr<fb::game::object> object);
    /**
     * @brief      Gets the unique identifier of this sector.
     *
     * @return     The sector's unique identifier.
     */
    uint32_t id() const;
    /**
     * @brief      Checks if this sector is currently active.
     *
     *             A sector is considered active when it contains at least one character,
     *             which determines whether the sector needs regular processing updates.
     *
     * @return     True if the sector is active (contains characters), false otherwise.
     */
    bool is_active() const;
};

/**
 * @brief      Manages a collection of sectors for spatial partitioning of a map.
 *
 *             This class provides a spatial partitioning system that divides a map
 *             into a grid of sectors for efficient object management and queries.
 *             It supports sector-based operations like finding nearby objects,
 *             managing active sectors, and optimizing spatial calculations.
 *
 *             Key features:
 *             - Grid-based spatial partitioning of maps
 *             - Efficient neighbor sector lookup
 *             - Object type filtering and queries
 *             - Active sector management for performance optimization
 *             - Position-to-sector mapping and indexing
 */
class sectors
{
public:
    using shared_sectors = std::vector<std::shared_ptr<sector>>;

private:
    const fb::model::size16_t _map_size = fb::model::size16_t(0, 0);
    const fb::model::size16_t _size     = fb::model::size16_t(0, 0);
    const uint32_t            _rows = 0, _columns = 0;
    const uint32_t            _count = 0;
    shared_sectors            _pool;

public:
    /**
     * @brief      Constructs a sectors grid for the specified map dimensions.
     *
     *             Creates a spatial partitioning grid that divides the given map
     *             into sectors of the specified size, initializing all necessary
     *             data structures for efficient spatial operations.
     *
     * @param[in]  map_size  The total dimensions of the map to partition.
     * @param[in]  size      The dimensions of each individual sector.
     */
    sectors(const fb::model::size16_t& map_size, const fb::model::size16_t& size);

    /**
     * @brief      Copy constructor is deleted to prevent accidental copying.
     */
    sectors(const sectors&) = delete;

    /**
     * @brief      Move constructor is deleted to prevent resource transfer issues.
     */
    sectors(sectors&&) = delete;

    /**
     * @brief      Destroys the sectors grid and cleans up resources.
     */
    ~sectors() = default;

public:
    /**
     * @brief      Assignment operator (deleted).
     *
     * @param      other  The other sectors object to assign from.
     *
     * @return     The result of the assignment
     */
    sectors& operator= (sectors& other) = delete;
    /**
     * @brief      Copy assignment operator (deleted).
     *
     * @param[in]  other  The other sectors object to copy assign from.
     *
     * @return     The result of the assignment
     */
    sectors& operator= (const sectors& other) = delete;

private:
    /**
     * @brief      Calculates the sector index for a given map position.
     *
     *             Converts a 2D map coordinate into a linear sector index for
     *             efficient sector lookup and management operations.
     *
     * @param[in]  position  The map position to convert to sector index.
     *
     * @return     The calculated sector index for the given position.
     */
    uint32_t index(const fb::model::point16_t& position) const;

    /**
     * @brief      Gets a set of all currently active sectors.
     *
     *             Returns a collection of sectors that are currently active
     *             (containing at least one character) for processing optimization.
     *
     * @return     Set of pointers to active sectors.
     */
    std::set<std::shared_ptr<fb::game::sector>> active_sectors() const;

public:
    /**
     * @brief      Gets the sector containing the specified map position.
     *
     *             Retrieves the sector that contains the given map coordinates,
     *             allowing for position-based sector queries and operations.
     *
     * @param[in]  position  The map position to find the sector for.
     *
     * @return     Pointer to the sector containing the position, or nullptr if invalid.
     */
    std::shared_ptr<fb::game::sector> at(const fb::model::point16_t& position) const;

    /**
     * @brief      Gets the sector at the specified index.
     *
     *             Retrieves the sector at the given linear index within the
     *             sectors grid for direct sector access operations.
     *
     * @param[in]  index  The sector index to retrieve.
     *
     * @return     Pointer to the sector at the specified index, or nullptr if invalid.
     */
    std::shared_ptr<fb::game::sector> at(uint32_t index) const;

public:
    /**
     * @brief      Gets all sectors neighboring the sector at the specified index.
     *
     *             Returns a collection of sectors that are adjacent to the sector
     *             at the given index, useful for area-of-effect operations and
     *             visibility calculations.
     *
     * @param[in]  index  The sector index to find neighbors for.
     *
     * @return     Vector of pointers to neighboring sectors.
     */
    std::vector<std::shared_ptr<fb::game::sector>> nears(uint32_t index) const;

    /**
     * @brief      Gets all sectors near the specified map position.
     *
     *             Returns a collection of sectors that are adjacent to the sector
     *             containing the given position, enabling efficient area queries.
     *
     * @param[in]  pivot  The map position to find nearby sectors for.
     *
     * @return     Vector of pointers to sectors near the specified position.
     */
    std::vector<std::shared_ptr<fb::game::sector>> nears(const fb::model::point16_t& pivot) const;

    /**
     * @brief      Gets all objects of a specific type near the given position.
     *
     *             Searches sectors near the specified position for objects matching
     *             the given type, enabling efficient spatial object queries.
     *
     * @param[in]  pivot  The map position to search around.
     * @param[in]  type   The type of objects to search for.
     *
     * @return     Vector of pointers to matching objects near the position.
     */
    std::vector<std::shared_ptr<fb::game::object>> objects(const fb::model::point16_t& pivot, OBJECT_TYPE type) const;

    /**
     * @brief      Gets all objects of a specific type across all sectors.
     *
     *             Searches all sectors for objects matching the given type,
     *             providing a comprehensive object query across the entire map.
     *
     * @param[in]  type  The type of objects to search for.
     *
     * @return     Vector of pointers to all matching objects in all sectors.
     */
    std::vector<std::shared_ptr<fb::game::object>> objects(OBJECT_TYPE type) const;

    /**
     * @brief      Checks if any sectors are currently active.
     *
     *             Determines whether there are any active sectors (containing
     *             characters) in the entire sectors grid.
     *
     * @return     True if at least one sector is active, false otherwise.
     */
    bool is_active() const;
};

} // namespace fb::game

#endif // !__SECTOR_H__
