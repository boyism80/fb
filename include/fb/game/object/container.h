#ifndef __OBJECT_CONTAINER_H__
#define __OBJECT_CONTAINER_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include <zlib.h>
#include <memory>
#include <queue>
#include <fb/game/door.h>
#include <fb/game/sector.h>
#include <fb/stream.h>
#include <fb/config.h>
#include <fb/container.h>
#include <async/awaitable_then.h>

using namespace std::chrono_literals;

namespace fb::game {

class object;
class map;
class mob;
class character;
class context;

/**
 * @brief      Container for managing game objects within a map.
 *
 *             This class provides a comprehensive container system for managing
 *             all game objects within a specific map. It handles object storage,
 *             retrieval, iteration, and lifecycle management with efficient
 *             lookup capabilities and type-based filtering.
 *
 *             Key features:
 *             - Unique object ID generation for objects
 *             - Efficient object lookup by ID and reference
 *             - Type-based object iteration and filtering
 *             - STL-compatible iterator interface
 *             - Safe object insertion and removal operations
 *             - Memory management with unique pointers
 */
class object_container
{
public:
    using ptrs     = std::unordered_map<uint32_t, std::shared_ptr<object>>; ///< Map of object IDs to shared pointers
    using iterator = ptrs::iterator;                            ///< Iterator type for object shared pointers
    using const_iterator        = ptrs::const_iterator;         ///< Const iterator type for object shared pointers
    using handle_predicate_type = std::function<bool(object&)>; ///< Function type for object filtering predicates

private:
    ptrs                 _ptrs;          ///< Storage for object shared pointers
    uint32_t             _oid = 1;       ///< Next available object ID for new objects
    std::queue<uint32_t> _available_seq; ///< Queue for reusing object IDs

public:
    fb::game::map& owner; ///< Reference to the map that owns this container

public:
    /**
     * @brief      Constructs a new object container for the specified map.
     *
     *             Initializes an empty object container associated with the given
     *             map, ready to manage objects within that map's boundaries.
     *
     * @param      map   The map that will own this object container.
     */
    object_container(fb::game::map& map);

    /**
     * @brief      Destroys the object container and cleans up resources.
     */
    ~object_container() = default;

private:
    /**
     * @brief      Allocates the next available object ID for a new object.
     *
     *             Returns a reusable object ID from the queue if available,
     *             otherwise generates a new unused object ID for assignment
     *             to a new object being added to the container.
     *
     * @return     The allocated object ID.
     */
    uint32_t allocate_seq();

public:
    /**
     * @brief      Gets an iterator to the beginning of the object collection.
     *
     *             Returns a mutable iterator pointing to the first object in the
     *             container for iteration and modification operations.
     *
     * @return     Iterator to the beginning of the object collection.
     */
    iterator begin();

    /**
     * @brief      Gets an iterator to the end of the object collection.
     *
     *             Returns a mutable iterator pointing past the last object in the
     *             container for iteration bounds checking.
     *
     * @return     Iterator to the end of the object collection.
     */
    iterator end();

    /**
     * @brief      Gets a const iterator to the beginning of the object collection.
     *
     *             Returns a read-only iterator pointing to the first object in the
     *             container for safe iteration without modification.
     *
     * @return     Const iterator to the beginning of the object collection.
     */
    const_iterator begin() const;

    /**
     * @brief      Gets a const iterator to the end of the object collection.
     *
     *             Returns a read-only iterator pointing past the last object in the
     *             container for safe iteration bounds checking.
     *
     * @return     Const iterator to the end of the object collection.
     */
    const_iterator end() const;

    /**
     * @brief      Gets the number of objects currently in the container.
     *
     *             Returns the total count of objects managed by this container,
     *             useful for capacity planning and iteration bounds.
     *
     * @return     The number of objects in the container.
     */
    uint32_t size() const;
    /**
     * @brief      Gets the object with the specified ID.
     *
     *             Retrieves the object associated with the given object ID,
     *             throwing an exception if the object is not found.
     *
     * @param[in]  i     The object ID of the object to retrieve.
     *
     * @return     Reference to the object with the specified ID.
     */
    std::shared_ptr<object> at(uint32_t i);

    /**
     * @brief      Adds an object to the container.
     *
     *             Inserts the specified object into the container, assigning it
     *             a unique object ID and making it available for lookup.
     *
     * @param      obj   The object to add to the container.
     */
    void push(object& obj);

    /**
     * @brief      Removes and returns the object with the specified ID.
     *
     *             Removes the object from the container and returns a reference
     *             to it, transferring ownership back to the caller.
     *
     * @param[in]  fd    The object ID of the object to remove.
     *
     * @return     Reference to the removed object.
     */
    object& pop(uint32_t fd);

    /**
     * @brief      Removes and returns the specified object.
     *
     *             Removes the given object from the container and returns a
     *             reference to it, transferring ownership back to the caller.
     *
     * @param      obj   The object to remove from the container.
     *
     * @return     Reference to the removed object.
     */
    object& pop(object& obj);
    /**
     * @brief      Safely attempts to remove the object with the specified ID.
     *
     *             Tries to remove the object from the container, returning a pointer
     *             to it if successful, or nullptr if the object is not found.
     *
     * @param[in]  fd    The object ID of the object to remove.
     *
     * @return     Pointer to the removed object, or nullptr if not found.
     */
    object* try_pop(uint32_t fd);

    /**
     * @brief      Safely attempts to remove the specified object.
     *
     *             Tries to remove the given object from the container, returning a
     *             pointer to it if successful, or nullptr if the object is not found.
     *
     * @param      obj   The object to remove from the container.
     *
     * @return     Pointer to the removed object, or nullptr if not found.
     */
    object* try_pop(object& obj);

    /**
     * @brief      Iterates over all objects of a specific type.
     *
     *             Applies the given predicate function to all objects in the container
     *             that match the specified type, enabling type-filtered operations.
     *
     * @param[in]  type  The type of objects to iterate over.
     * @param[in]  fn    The predicate function to apply to each matching object.
     */
    void foreach (OBJECT_TYPE type, const handle_predicate_type& fn);

    /**
     * @brief      Checks if the container contains the specified object.
     *
     *             Determines whether the given object is currently managed by
     *             this container.
     *
     * @param[in]  obj   The object to check for containment.
     *
     * @return     True if the object is in the container, false otherwise.
     */
    bool contains(const std::shared_ptr<object>& obj) const;

    /**
     * @brief      Checks if the container contains an object with the specified ID.
     *
     *             Determines whether an object with the given object ID is
     *             currently managed by this container.
     *
     * @param[in]  fd    The object ID to check for.
     *
     * @return     True if an object with the ID exists, false otherwise.
     */
    bool contains(uint32_t fd) const;

public:
    /**
     * @brief      Array indexer operator for object lookup.
     *
     *             Provides convenient array-style access to objects by their
     *             object ID, returning nullptr if the object is not found.
     *
     * @param[in]  fd    The object ID of the object to retrieve.
     *
     * @return     Pointer to the object with the specified ID, or nullptr if not found.
     */
    object* operator[] (uint32_t fd);
};

} // namespace fb::game

#endif