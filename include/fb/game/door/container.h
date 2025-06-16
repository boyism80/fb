#ifndef __DOOR_CONTAINER_H__
#define __DOOR_CONTAINER_H__

#include <fb/game/door.h>

namespace fb::game {

/**
 * @brief      Container for managing doors within a map.
 *
 *             This class provides a specialized container for managing door objects
 *             within a specific map. It handles door storage, retrieval, and provides
 *             efficient lookup capabilities for door-related operations such as
 *             character interaction and map transitions.
 *
 *             Key features:
 *             - Position-based door lookup and management
 *             - Character-specific door finding for interactions
 *             - Custom iterators for door enumeration
 *             - Integration with map boundaries and positioning
 *             - Shared pointer management for door lifecycle
 */
class door_container : private std::unordered_map<uint64_t, std::shared_ptr<fb::game::door>>
{
private:
    using super = std::unordered_map<uint64_t, std::shared_ptr<fb::game::door>>;

public:
    /**
     * @brief      Iterator for traversing doors with position information.
     *
     *             Provides iteration over doors while maintaining position context
     *             for spatial operations and door management.
     */
    class iterator;

    /**
     * @brief      Const iterator for read-only door traversal with position information.
     *
     *             Provides read-only iteration over doors while maintaining position
     *             context for spatial queries and door inspection.
     */
    class const_iterator;

    using base_iterator       = super::iterator;       ///< Base iterator type from the underlying map
    using const_base_iterator = super::const_iterator; ///< Base const iterator type from the underlying map

    friend class iterator;
    friend class const_iterator;

public:
    const fb::game::map& map; ///< Reference to the map that owns this door container

public:
    using super::at;
    using super::begin;
    using super::cbegin;
    using super::cend;
    using super::contains;
    using super::end;
    using super::size;

public:
    /**
     * @brief      Constructs a new door container for the specified map.
     *
     *             Initializes an empty door container associated with the given
     *             map, ready to manage doors within that map's boundaries.
     *
     * @param[in]  map   The map that will own this door container.
     */
    door_container(const fb::game::map& map);

    /**
     * @brief      Copy constructor is deleted to prevent accidental copying.
     */
    door_container(const door_container&) = delete;

    /**
     * @brief      Destroys the door container and cleans up resources.
     */
    ~door_container();

public:
    /**
     * @brief      Gets an iterator to the beginning of the door collection.
     *
     *             Returns a mutable iterator pointing to the first door in the
     *             container with position information for spatial operations.
     *
     * @return     Iterator to the beginning of the door collection.
     */
    iterator begin();

    /**
     * @brief      Gets an iterator to the end of the door collection.
     *
     *             Returns a mutable iterator pointing past the last door in the
     *             container for iteration bounds checking.
     *
     * @return     Iterator to the end of the door collection.
     */
    iterator end();

    /**
     * @brief      Gets a const iterator to the beginning of the door collection.
     *
     *             Returns a read-only iterator pointing to the first door in the
     *             container with position information for spatial queries.
     *
     * @return     Const iterator to the beginning of the door collection.
     */
    const_iterator begin() const;

    /**
     * @brief      Gets a const iterator to the end of the door collection.
     *
     *             Returns a read-only iterator pointing past the last door in the
     *             container for safe iteration bounds checking.
     *
     * @return     Const iterator to the end of the door collection.
     */
    const_iterator end() const;

    /**
     * @brief      Adds a door to the container.
     *
     *             Inserts the specified door into the container, making it available
     *             for character interactions and map transitions.
     *
     * @param      door  The shared pointer to the door to add.
     */
    void add(std::shared_ptr<door>& door);

    /**
     * @brief      Finds a door that the character can interact with.
     *
     *             Searches for a door that is accessible to the specified character
     *             based on their current position and interaction range.
     *
     * @param[in]  ch    The character to find an accessible door for.
     *
     * @return     Pointer to an accessible door, or nullptr if none found.
     */
    door* find(const character& ch) const;
};

/**
 * @brief      This class describes an iterator.
 */
class door_container::iterator : public door_container::base_iterator
{
public:
    std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new iterator instance.
     *
     * @param[in]  i          The base iterator to wrap.
     * @param[in]  container  The door container being iterated over.
     */
    iterator(const door_container::base_iterator& i, const door_container& container);
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
    std::pair<fb::model::point16_t, door&> operator* ();
};

/**
 * @brief      This class describes a constant iterator.
 */
class door_container::const_iterator : public door_container::const_base_iterator
{
public:
    const std::optional<std::pair<fb::model::point16_t, door&>> pair;

public:
    /**
     * @brief      Constructs a new const iterator instance.
     *
     * @param[in]  i          The base const iterator to wrap.
     * @param[in]  container  The door container being iterated over.
     */
    const_iterator(const door_container::const_base_iterator& i, const door_container& container);
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
    const std::pair<fb::model::point16_t, door&> operator* () const;
};

} // namespace fb::game

#endif