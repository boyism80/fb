#include <fb/game/container.h>

template<typename T>
inline fb::game::base_container<T>::base_container(life& owner) : _owner(owner)
{ }
template<typename T>
inline fb::game::base_container<T>::~base_container()
{ }
template<typename T>
inline T* fb::game::base_container<T>::set(T* element, int position)
{
    auto before = this->_elements[position];
    this->_elements[position] = element;
    return before;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::next() const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->at(i) == nullptr)
            return i;
    }

    return 0xFF;
}
template<typename T>
inline fb::game::life& fb::game::base_container<T>::owner()
{
    return this->_owner;
}
template<typename T>
inline const fb::game::life& fb::game::base_container<T>::owner() const
{
    return this->_owner;
}
template<typename T>
inline T* fb::game::base_container<T>::at(uint8_t index) const
{
    if(index > CONTAINER_CAPACITY - 1)
        return nullptr;

    return this->_elements[index];
}
template<typename T>
inline uint8_t fb::game::base_container<T>::add(T& element)
{
    auto next = this->next();
    if(next != 0xFF)
    {
        this->_elements[next] = &element;
    }

    return next;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::add(T& element, uint8_t index)
{
    if(this->_elements[index] != nullptr)
        return 0xFF;

    this->_elements[index] = &element;
    return index;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::add(T* element, uint8_t index)
{
    return this->add(*element, index);
}
template<typename T>
inline uint8_t fb::game::base_container<T>::add(T* element)
{
    return this->add(*element);
}
template<typename T>
inline bool fb::game::base_container<T>::remove(uint8_t index)
{
    auto                    element = this->at(index);
    if(element == nullptr)
        return false;

    this->_elements[index] = nullptr;
    return true;
}
template<typename T>
inline bool fb::game::base_container<T>::swap(uint8_t src, uint8_t dst)
{
    if(src == dst)
        return false;

    if(src > CONTAINER_CAPACITY - 1 || dst > CONTAINER_CAPACITY - 1)
        return false;

    std::swap(this->_elements[src], this->_elements[dst]);
    return true;
}
template<typename T>
inline bool fb::game::base_container<T>::free() const
{
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->_elements[i] == nullptr)
            return true;
    }

    return false;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::free_size() const
{
    uint8_t                 count = 0;
    for(int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        if(this->_elements[i] == nullptr)
            count++;
    }

    return count;
}
template<typename T>
inline T* fb::game::base_container<T>::operator[](int index)
{
    return this->at(index);
}
template<typename T>
inline const T* fb::game::base_container<T>::operator[](int index) const
{
    return this->at(index);
}