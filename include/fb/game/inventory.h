#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include <stdlib.h>
#include <memory>
#include <array>
#include <async/task.h>

namespace fb { namespace game {

static constexpr uint8_t CONTAINER_CAPACITY = 52;

template <typename T>
class inventory
{
public:
    using array_type     = std::array<std::shared_ptr<T>, CONTAINER_CAPACITY>;
    using iterator       = typename array_type::iterator;
    using const_iterator = typename array_type::const_iterator;

private:
    array_type _elements = {};

protected:
    inventory() = default;

    ~inventory() = default;

protected:
    std::shared_ptr<T> set(std::shared_ptr<T> element, int position)
    {
        auto before               = this->_elements[position];
        this->_elements[position] = element;
        return before;
    }

public:
    uint8_t next() const
    {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (this->at(i) == nullptr)
                return i;
        }
        return 0xFF;
    }

    virtual std::shared_ptr<T> at(uint8_t index) const
    {
        if (index > CONTAINER_CAPACITY - 1)
            return nullptr;
        return this->_elements[index];
    }

    virtual async::task<uint8_t> add(std::shared_ptr<T> element)
    {
        auto next_slot = this->next();
        if (next_slot != 0xFF)
        {
            this->_elements[next_slot] = element;
        }
        co_return next_slot;
    }

    virtual async::task<uint8_t> add(std::shared_ptr<T> element, uint8_t index)
    {
        if (this->_elements[index] != nullptr)
            co_return 0xFF;
        this->_elements[index] = element;
        co_return index;
    }

    virtual async::task<bool> remove(uint8_t index)
    {
        auto element = this->at(index);
        if (element == nullptr)
            co_return false;
        this->_elements[index] = nullptr;
        co_return true;
    }

    virtual async::task<bool> swap(uint8_t src, uint8_t dst)
    {
        if (src == dst)
            co_return false;
        if (src > CONTAINER_CAPACITY - 1 || dst > CONTAINER_CAPACITY - 1)
            co_return false;
        std::swap(this->_elements[src], this->_elements[dst]);
        co_return true;
    }

    bool free() const
    {
        for (int i = 0; i < CONTAINER_CAPACITY; i++)
        {
            if (this->_elements[i] == nullptr)
                return true;
        }
        return false;
    }

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

    iterator begin()
    {
        return this->_elements.begin();
    }

    iterator end()
    {
        return this->_elements.end();
    }

    const_iterator cbegin() const
    {
        return this->_elements.cbegin();
    }

    const_iterator cend() const
    {
        return this->_elements.cend();
    }

public:
    std::shared_ptr<T> operator[] (int index)
    {
        return this->at(index);
    }

    const std::shared_ptr<T> operator[] (int index) const
    {
        return this->at(index);
    }
};

}} // namespace fb::game

#endif // !__CONTAINER_H__
