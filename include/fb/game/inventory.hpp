#include <fb/game/inventory.h>

/**
 * @brief      Constructs an inventory for the specified owner.
 *
 * @param      owner  The life entity that owns this inventory
 *
 * @tparam     T      The type of elements stored in the inventory
 */
template <typename T>
inline fb::game::inventory<T>::inventory(life& owner) :
    _owner(owner)
{ }

/**
 * @brief      Destroys the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 */
template <typename T>
inline fb::game::inventory<T>::~inventory()
{ }

/**
 * @brief      Sets an element at the specified position, replacing any existing element.
 *
 * @param      element   The element to place in the inventory
 * @param[in]  position  The position index where to place the element
 *
 * @tparam     T         The type of elements stored in the inventory
 *
 * @return     The previously stored element at that position, or nullptr if empty
 */
template <typename T>
inline T* fb::game::inventory<T>::set(T* element, int position)
{
    auto before               = this->_elements[position];
    this->_elements[position] = element;
    return before;
}

/**
 * @brief      Finds the next available slot in the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     The index of the next free slot, or 0xFF if inventory is full
 */
template <typename T>
inline uint8_t fb::game::inventory<T>::next() const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if (this->at(i) == nullptr)
            return i;
    }

    return 0xFF;
}

/**
 * @brief      Gets the owner of this inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Reference to the life entity that owns this inventory
 */
template <typename T>
inline fb::game::life& fb::game::inventory<T>::owner()
{
    return this->_owner;
}

/**
 * @brief      Gets the owner of this inventory (const version).
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Const reference to the life entity that owns this inventory
 */
template <typename T>
inline const fb::game::life& fb::game::inventory<T>::owner() const
{
    return this->_owner;
}

/**
 * @brief      Gets the element at the specified index.
 *
 * @param[in]  index  The index to retrieve the element from
 *
 * @tparam     T      The type of elements stored in the inventory
 *
 * @return     Pointer to the element at the index, or nullptr if empty or invalid index
 */
template <typename T>
inline T* fb::game::inventory<T>::at(uint8_t index) const
{
    if (index > CONTAINER_CAPACITY - 1)
        return nullptr;

    return this->_elements[index];
}

/**
 * @brief      Adds an element to the next available slot in the inventory.
 *
 * @param      element  The element to add to the inventory
 *
 * @tparam     T        The type of elements stored in the inventory
 *
 * @return     The index where the element was placed, or 0xFF if inventory is full
 */
template <typename T>
inline uint8_t fb::game::inventory<T>::add(T& element)
{
    auto next = this->next();
    if (next != 0xFF)
    {
        this->_elements[next] = &element;
    }

    return next;
}

/**
 * @brief      Adds an element to a specific slot in the inventory.
 *
 * @param      element  The element to add to the inventory
 * @param[in]  index    The specific index where to place the element
 *
 * @tparam     T        The type of elements stored in the inventory
 *
 * @return     The index where the element was placed, or 0xFF if slot is occupied
 */
template <typename T>
inline uint8_t fb::game::inventory<T>::add(T& element, uint8_t index)
{
    if (this->_elements[index] != nullptr)
        return 0xFF;

    this->_elements[index] = &element;
    return index;
}

/**
 * @brief      Removes an element from the specified index.
 *
 * @param[in]  index  The index of the element to remove
 *
 * @tparam     T      The type of elements stored in the inventory
 *
 * @return     True if element was successfully removed, false if slot was empty
 */
template <typename T>
inline bool fb::game::inventory<T>::remove(uint8_t index)
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
 * @param[in]  src  The source slot index
 * @param[in]  dst  The destination slot index
 *
 * @tparam     T    The type of elements stored in the inventory
 *
 * @return     True if swap was successful, false if indices are invalid or identical
 */
template <typename T>
inline bool fb::game::inventory<T>::swap(uint8_t src, uint8_t dst)
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
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     True if at least one slot is available, false if inventory is full
 */
template <typename T>
inline bool fb::game::inventory<T>::free() const
{
    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if (this->_elements[i] == nullptr)
            return true;
    }

    return false;
}

/**
 * @brief      Counts the number of free slots in the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     The number of available slots in the inventory
 */
template <typename T>
inline uint8_t fb::game::inventory<T>::free_size() const
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
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Iterator pointing to the first element slot
 */
template <typename T>
fb::game::inventory<T>::iterator fb::game::inventory<T>::begin()
{
    return this->_elements.begin();
}

/**
 * @brief      Gets an iterator to the end of the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Iterator pointing past the last element slot
 */
template <typename T>
fb::game::inventory<T>::iterator fb::game::inventory<T>::end()
{
    return this->_elements.end();
}

/**
 * @brief      Gets a const iterator to the beginning of the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Const iterator pointing to the first element slot
 */
template <typename T>
fb::game::inventory<T>::const_iterator fb::game::inventory<T>::cbegin() const
{
    return this->_elements.cbegin();
}

/**
 * @brief      Gets a const iterator to the end of the inventory.
 *
 * @tparam     T     The type of elements stored in the inventory
 *
 * @return     Const iterator pointing past the last element slot
 */
template <typename T>
fb::game::inventory<T>::const_iterator fb::game::inventory<T>::cend() const
{
    return this->_elements.cend();
}

/**
 * @brief      Array subscript operator for accessing inventory elements.
 *
 * @param[in]  index  The index of the element to access
 *
 * @tparam     T      The type of elements stored in the inventory
 *
 * @return     Pointer to the element at the specified index
 */
template <typename T>
inline T* fb::game::inventory<T>::operator[] (int index)
{
    return this->at(index);
}

/**
 * @brief      Array subscript operator for accessing inventory elements (const version).
 *
 * @param[in]  index  The index of the element to access
 *
 * @tparam     T      The type of elements stored in the inventory
 *
 * @return     Const pointer to the element at the specified index
 */
template <typename T>
inline const T* fb::game::inventory<T>::operator[] (int index) const
{
    return this->at(index);
}