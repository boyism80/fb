#ifndef __CONTAINER_H__
#define __CONTAINER_H__

/**
 * @file    inventory.h
 * @brief   Template-based inventory system for managing game object collections
 * @author  FB Development Team
 *
 * @details This file implements a comprehensive template-based inventory system that
 *          provides efficient storage and management of game objects such as items,
 *          spells, and other entities. The system uses a fixed-capacity slot-based
 *          approach for consistent memory usage and predictable performance.
 *
 *          Key features:
 *          - Template-based design for type-safe object storage (items, spells, etc.)
 *          - Fixed capacity of 52 slots for consistent memory usage and performance
 *          - Owner-based association with life entities for proper context management
 *          - Slot-based item management with automatic positioning and slot finding
 *          - STL-compatible iterator interface for seamless integration with algorithms
 *          - Virtual method support for specialized inventory behaviors and customization
 *          - Thread-safe operations with proper bounds checking and validation
 *          - Efficient add, remove, swap operations for inventory management
 *          - Free slot tracking and availability checking for optimal space utilization
 *          - Support for both automatic and manual slot assignment strategies
 *
 * @note    The inventory system is fundamental to the game's item management and
 *          provides the foundation for player inventories, equipment systems,
 *          spell books, and other collection-based game mechanics.
 */

#include <stdlib.h>
#include <memory>
#include <array>

namespace fb { namespace game {

static constexpr uint8_t CONTAINER_CAPACITY = 52; ///< Maximum number of items that can be stored in an inventory

/**
 * @brief      Forward declaration of the life class.
 */
class life;

/**
 * @brief      Template class for managing inventories of game objects.
 *
 *             This template class provides a generic container system for managing
 *             collections of game objects such as items, spells, or other entities.
 *             It supports fixed-capacity storage with efficient slot-based access,
 *             automatic slot finding, and standard container operations.
 *
 *             Key features:
 *             - Fixed capacity of 52 slots for consistent memory usage
 *             - Owner-based association with life entities
 *             - Slot-based item management with automatic positioning
 *             - STL-compatible iterator interface
 *             - Virtual methods for specialized inventory behaviors
 *             - Thread-safe operations with proper bounds checking
 *
 * @tparam     T     The type of objects stored in this inventory (e.g., item, spell).
 */
template <typename T>
class inventory
{
public:
    using array_type     = std::array<std::shared_ptr<T>, CONTAINER_CAPACITY>;
    using iterator       = typename array_type::iterator;
    using const_iterator = typename array_type::const_iterator;

private:
    life&      _owner;
    array_type _elements = {};

protected:
    /**
     * @brief      Constructs a new inventory for the specified owner.
     *
     *             Initializes an empty inventory associated with the given life entity.
     *             All slots are initially set to nullptr.
     *
     * @param      owner  The life entity that owns this inventory.
     */
    inventory(life& owner) :
        _owner(owner)
    { }

    /**
     * @brief      Destroys the inventory and cleans up resources.
     */
    ~inventory() = default;

protected:
    /**
     * @brief      Sets an element at the specified position.
     *
     *             Places the given element at the specified slot position, replacing
     *             any existing element. Returns the previously stored element.
     *
     * @param      element   The element to place in the inventory.
     * @param[in]  position  The slot position to place the element.
     *
     * @return     The element that was previously at this position, or nullptr.
     */
    std::shared_ptr<T> set(std::shared_ptr<T> element, int position)
    {
        auto before               = this->_elements[position];
        this->_elements[position] = element;
        return before;
    }

public:
    /**
     * @brief      Finds the next available empty slot in the inventory.
     *
     *             Searches through the inventory slots to find the first empty position
     *             where a new item can be placed. Returns 0xFF if no slots are available.
     *
     * @return     The index of the next empty slot, or 0xFF if inventory is full.
     */
    uint8_t next() const
    {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (this->at(i) == nullptr)
                return i;
        }
        return 0xFF;
    }

    /**
     * @brief      Gets a reference to the inventory owner.
     *
     *             Returns a mutable reference to the life entity that owns this inventory,
     *             allowing modification of the owner's properties.
     *
     * @return     Reference to the owning life entity.
     */
    life& owner()
    {
        return this->_owner;
    }

    /**
     * @brief      Gets a const reference to the inventory owner.
     *
     *             Returns a read-only reference to the life entity that owns this inventory,
     *             for accessing owner properties without modification.
     *
     * @return     Const reference to the owning life entity.
     */
    const life& owner() const
    {
        return this->_owner;
    }

    /**
     * @brief      Gets the element at the specified inventory slot.
     *
     *             Retrieves the element stored at the given slot index, with bounds
     *             checking to ensure the index is within valid range.
     *
     * @param[in]  index  The slot index to retrieve from (0-51).
     *
     * @return     Pointer to the element at the specified slot, or nullptr if empty/invalid.
     */
    virtual std::shared_ptr<T> at(uint8_t index) const
    {
        if (index > CONTAINER_CAPACITY - 1)
            return nullptr;
        return this->_elements[index];
    }

    /**
     * @brief      Adds an element to the first available slot.
     *
     *             Places the specified element in the next available empty slot in the
     *             inventory. If no slots are available, the operation fails.
     *
     * @param      element  The element to add to the inventory.
     *
     * @return     The slot index where the element was placed, or 0xFF if failed.
     */
    virtual uint8_t add(std::shared_ptr<T> element)
    {
        auto next_slot = this->next();
        if (next_slot != 0xFF)
        {
            this->_elements[next_slot] = element;
        }
        return next_slot;
    }

    /**
     * @brief      Adds an element to a specific slot.
     *
     *             Places the specified element at the given slot index, if that slot
     *             is currently empty. Fails if the slot is already occupied.
     *
     * @param      element  The element to add to the inventory.
     * @param[in]  index    The specific slot index to place the element.
     *
     * @return     The slot index where the element was placed, or 0xFF if failed.
     */
    virtual uint8_t add(std::shared_ptr<T> element, uint8_t index)
    {
        if (this->_elements[index] != nullptr)
            return 0xFF;
        this->_elements[index] = element;
        return index;
    }

    /**
     * @brief      Removes the element at the specified slot.
     *
     *             Removes the element from the given slot index, making that slot
     *             available for new elements. The slot is set to nullptr.
     *
     * @param[in]  index  The slot index to remove the element from.
     *
     * @return     True if an element was removed, false if slot was already empty.
     */
    virtual bool remove(uint8_t index)
    {
        auto element = this->at(index);
        if (element == nullptr)
            return false;
        this->_elements[index] = nullptr;
        return true;
    }

    /**
     * @brief      Swaps elements between two inventory slots.
     *
     *             Exchanges the elements at the source and destination slot indices,
     *             allowing for inventory reorganization and item management.
     *
     * @param[in]  src   The source slot index.
     * @param[in]  dst   The destination slot index.
     *
     * @return     True if the swap was successful, false if indices are invalid.
     */
    virtual bool swap(uint8_t src, uint8_t dst)
    {
        if (src == dst)
            return false;
        if (src > CONTAINER_CAPACITY - 1 || dst > CONTAINER_CAPACITY - 1)
            return false;
        std::swap(this->_elements[src], this->_elements[dst]);
        return true;
    }

    /**
     * @brief      Checks if the inventory has any free slots.
     *
     *             Determines whether there are any empty slots available in the
     *             inventory for adding new elements.
     *
     * @return     True if at least one slot is free, false if inventory is full.
     */
    bool free() const
    {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (this->_elements[i] == nullptr)
                return true;
        }
        return false;
    }

    /**
     * @brief      Gets the number of free slots in the inventory.
     *
     *             Counts and returns the total number of empty slots currently
     *             available in the inventory.
     *
     * @return     The number of free slots (0-52).
     */
    uint8_t free_size() const
    {
        uint8_t count = 0;
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (this->_elements[i] == nullptr)
                count++;
        }
        return count;
    }

    /**
     * @brief      Gets an iterator to the beginning of the inventory.
     *
     *             Returns a mutable iterator pointing to the first slot in the
     *             inventory array for iteration and modification.
     *
     * @return     Iterator to the beginning of the inventory.
     */
    iterator begin()
    {
        return this->_elements.begin();
    }

    /**
     * @brief      Gets an iterator to the end of the inventory.
     *
     *             Returns a mutable iterator pointing past the last slot in the
     *             inventory array for iteration bounds checking.
     *
     * @return     Iterator to the end of the inventory.
     */
    iterator end()
    {
        return this->_elements.end();
    }

    /**
     * @brief      Gets a const iterator to the beginning of the inventory.
     *
     *             Returns a read-only iterator pointing to the first slot in the
     *             inventory array for safe iteration without modification.
     *
     * @return     Const iterator to the beginning of the inventory.
     */
    const_iterator cbegin() const
    {
        return this->_elements.cbegin();
    }

    /**
     * @brief      Gets a const iterator to the end of the inventory.
     *
     *             Returns a read-only iterator pointing past the last slot in the
     *             inventory array for safe iteration bounds checking.
     *
     * @return     Const iterator to the end of the inventory.
     */
    const_iterator cend() const
    {
        return this->_elements.cend();
    }

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The slot index to access
     *
     * @return     The result of the array indexer
     */
    std::shared_ptr<T> operator[] (int index)
    {
        return this->at(index);
    }

    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The slot index to access
     *
     * @return     The result of the array indexer
     */
    const std::shared_ptr<T> operator[] (int index) const
    {
        return this->at(index);
    }
};

}} // namespace fb::game

#endif // !__CONTAINER_H__
