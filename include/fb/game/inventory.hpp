#include <fb/game/inventory.h>

template<typename T>
inline fb::game::inventory<T>::inventory(life& owner) : _owner(owner)
{ }

template<typename T>
inline fb::game::inventory<T>::~inventory()
{ }

template<typename T>
inline T* fb::game::inventory<T>::set(T* element, int position)
{
    auto before = this->_elements[position];
    this->_elements[position] = element;
    return before;
}

template<typename T>
inline uint8_t fb::game::inventory<T>::next() const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->at(i) == nullptr)
            return i;
    }

    return 0xFF;
}

template<typename T>
inline fb::game::life& fb::game::inventory<T>::owner()
{
    return this->_owner;
}

template<typename T>
inline const fb::game::life& fb::game::inventory<T>::owner() const
{
    return this->_owner;
}

template<typename T>
inline T* fb::game::inventory<T>::at(uint8_t index) const
{
    if(index > CONTAINER_CAPACITY - 1)
        return nullptr;

    return this->_elements[index];
}

template<typename T>
inline fb::task<uint8_t> fb::game::inventory<T>::add(T& element)
{
    auto next = this->next();
    if(next != 0xFF)
    {
        this->_elements[next] = &element;
    }

    co_return next;
}

template<typename T>
inline fb::task<uint8_t> fb::game::inventory<T>::add(T& element, uint8_t index)
{
    if(this->_elements[index] != nullptr)
        co_return 0xFF;

    this->_elements[index] = &element;
    co_return index;
}

template<typename T>
inline bool fb::game::inventory<T>::remove(uint8_t index)
{
    auto                    element = this->at(index);
    if(element == nullptr)
        return false;

    this->_elements[index] = nullptr;
    return true;
}

template<typename T>
inline bool fb::game::inventory<T>::swap(uint8_t src, uint8_t dst)
{
    if(src == dst)
        return false;

    if(src > CONTAINER_CAPACITY - 1 || dst > CONTAINER_CAPACITY - 1)
        return false;

    std::swap(this->_elements[src], this->_elements[dst]);
    return true;
}

template<typename T>
inline bool fb::game::inventory<T>::free() const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->_elements[i] == nullptr)
            return true;
    }

    return false;
}

template<typename T>
inline uint8_t fb::game::inventory<T>::free_size() const
{
    uint8_t                 count = 0;
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->_elements[i] == nullptr)
            count++;
    }

    return count;
}

template <typename T>
fb::game::inventory<T>::iterator fb::game::inventory<T>::begin()
{
    return this->_elements.begin();
}

template <typename T>
fb::game::inventory<T>::iterator fb::game::inventory<T>::end()
{
    return this->_elements.end();
}

template <typename T>
fb::game::inventory<T>::const_iterator fb::game::inventory<T>::cbegin() const
{
    return this->_elements.cbegin();
}

template <typename T>
fb::game::inventory<T>::const_iterator fb::game::inventory<T>::cend() const
{
    return this->_elements.cend();
}

template<typename T>
inline T* fb::game::inventory<T>::operator[](int index)
{
    return this->at(index);
}

template<typename T>
inline const T* fb::game::inventory<T>::operator[](int index) const
{
    return this->at(index);
}