#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdlib.h>
#include <memory>

namespace fb { namespace game {

static constexpr uint8_t CONTAINER_CAPACITY = 52;

/**
 * @brief      This class describes a life.
 */
class life;

/**
 * @brief      This class describes an inventory.
 *
 * @tparam     T     { description }
 */
template <typename T>
class inventory
{
public:
    using array_type     = std::array<T*, CONTAINER_CAPACITY>;
    using iterator       = array_type::iterator;
    using const_iterator = array_type::const_iterator;

private:
    life&      _owner;
    array_type _elements = {
        nullptr,
    };

protected:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      owner  The owner
     */
    inventory(life& owner);
    /**
     * @brief      Destroys the object.
     */
    ~inventory();

protected:
    /**
     * @brief      { function_description }
     *
     * @param      element   The element
     * @param[in]  position  The position
     *
     * @return     { description_of_the_return_value }
     */
    T* set(T* element, int position);

public:
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t next() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    life& owner();
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    const life& owner() const;
    /**
     * @brief      { function_description }
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    virtual T* at(uint8_t index) const;
    /**
     * @brief      Adds the specified element.
     *
     * @param      element  The element
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] virtual async::task<uint8_t> add(T& element);
    /**
     * @brief      { function_description }
     *
     * @param      element  The element
     * @param[in]  index    The index
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] virtual async::task<uint8_t> add(T& element, uint8_t index);
    /**
     * @brief      Removes the specified index.
     *
     * @param[in]  index  The index
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] virtual async::task<bool> remove(uint8_t index);
    /**
     * @brief      { function_description }
     *
     * @param[in]  src   The source
     * @param[in]  dst   The destination
     *
     * @return     { description_of_the_return_value }
     */
    [[nodiscard]] virtual async::task<bool> swap(uint8_t src, uint8_t dst);
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    bool free() const;
    /**
     * @brief      { function_description }
     *
     * @return     { description_of_the_return_value }
     */
    uint8_t free_size() const;
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
    const_iterator cbegin() const;
    /**
     * @brief      { function_description }
     *
     * @return     The constant iterator.
     */
    const_iterator cend() const;

public:
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The index
     *
     * @return     The result of the array indexer
     */
    T* operator[] (int index);
    /**
     * @brief      Array indexer operator.
     *
     * @param[in]  index  The index
     *
     * @return     The result of the array indexer
     */
    const T* operator[] (int index) const;
};

#include "inventory.hpp"

}} // namespace fb::game

#endif // !__CONTAINER_H__
