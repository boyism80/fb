#include "container.h"

template<typename T>
inline fb::game::container<T>::container(life& owner, uint8_t size, bool auto_release) : 
    _owner(owner),
    _size(size),
    _auto_release(auto_release)
{
    this->_elements = new T*[size];
    memset(this->_elements, NULL, sizeof(T*) * size);
}
template<typename T>
inline fb::game::container<T>::~container()
{
    if(this->_auto_release)
    {
        for(int i = 0; i < this->_size; i++)
            delete this->_elements[i];
    }

    delete[] this->_elements;
}
template<typename T>
inline T* fb::game::container<T>::set(T* element, int position)
{
    auto before = this->_elements[position];
    this->_elements[position] = element;
    return before;
}
template<typename T>
inline fb::game::life& fb::game::container<T>::owner()
{
    return this->_owner;
}
template<typename T>
inline const fb::game::life& fb::game::container<T>::owner() const
{
    return this->_owner;
}
template<typename T>
inline T* fb::game::container<T>::at(uint8_t index) const
{
    if(index > this->_size - 1)
        return nullptr;

    return this->_elements[index];
}
template<typename T>
inline bool fb::game::container<T>::add(T& element)
{
    for(int i = 0; i < this->_size; i++)
    {
        if(this->_elements[i] != nullptr)
            continue;

        this->_elements[i] = &element;
        return true;
    }

    return false;
}
template<typename T>
inline uint8_t fb::game::container<T>::add(T* element)
{
    return this->add(*element);
}
template<typename T>
inline bool fb::game::container<T>::remove(uint8_t index)
{
    auto                    element = this->at(index);
    if(element == nullptr)
        return false;

    this->_elements[index] = nullptr;
    return true;
}
template<typename T>
inline bool fb::game::container<T>::swap(uint8_t src, uint8_t dest)
{
    if(src == dest)
        return false;

    if(src > this->_size - 1 || dest > this->_size - 1)
        return false;

    std::swap(this->_elements[src], this->_elements[dest]);
    return true;
}
template<typename T>
inline bool fb::game::container<T>::free() const
{
    for(int i = 0; i < this->_size; i++)
    {
        if(this->_elements[i] == nullptr)
            return true;
    }

    return false;
}
template<typename T>
inline uint8_t fb::game::container<T>::free_size() const
{
    uint8_t                 count = 0;
    for(int i = 0; i < this->_size; i++)
    {
        if(this->_elements[i] == nullptr)
            count++;
    }

    return count;
}
template<typename T>
inline T* fb::game::container<T>::operator[](int index)
{
    return this->at(index);
}
template<typename T>
inline const T* fb::game::container<T>::operator[](int index) const
{
    return this->at(index);
}