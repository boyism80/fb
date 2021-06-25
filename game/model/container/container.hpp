#include "model/container/container.h"

template<typename T>
inline fb::game::base_container<T>::base_container(life& owner, uint8_t size, bool auto_release) : 
    _owner(owner),
    _size(size),
    _auto_release(auto_release)
{
    this->_elements = new T*[size];
    std::memset(this->_elements, 0, sizeof(T*) * size);
}
template<typename T>
inline fb::game::base_container<T>::~base_container()
{
    if(this->_auto_release)
    {
        for(int i = 0; i < this->_size; i++)
            delete this->_elements[i];
    }

    delete[] this->_elements;
}
template<typename T>
inline T* fb::game::base_container<T>::set(T* element, int position)
{
    auto before = this->_elements[position];
    this->_elements[position] = element;
    this->modify(position);
    return before;
}
template<typename T>
inline void fb::game::base_container<T>::modify(uint8_t index)
{
    this->_modified = this->_modified | (static_cast<unsigned long long>(0x0000000000000001) << index);
}
template<typename T>
inline bool fb::game::base_container<T>::modified() const
{
    return this->_modified != 0x0000000000000000;
}
template<typename T>
inline bool fb::game::base_container<T>::modified(uint8_t index) const
{
    auto flag = (static_cast<unsigned long long>(0x0000000000000001) << index);
    return (this->_modified & flag) == flag;
}
template<typename T>
inline bool fb::game::base_container<T>::modified_reset()
{
    this->_modified = 0x0000000000000000;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::next() const
{
    for(int i = 0; i < this->_size; i++)
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
    if(index > this->_size - 1)
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
        this->modify(next);
    }

    return next;
}
template<typename T>
inline uint8_t fb::game::base_container<T>::add(T& element, uint8_t index)
{
    if(this->_elements[index] != nullptr)
        return 0xFF;

    this->_elements[index] = &element;
    this->modify(index);
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
    this->modify(index);
    return true;
}
template<typename T>
inline bool fb::game::base_container<T>::swap(uint8_t src, uint8_t dst)
{
    if(src == dst)
        return false;

    if(src > this->_size - 1 || dst > this->_size - 1)
        return false;

    std::swap(this->_elements[src], this->_elements[dst]);
    this->modify(src);
    this->modify(dst);
    return true;
}
template<typename T>
inline bool fb::game::base_container<T>::free() const
{
    for(int i = 0; i < this->_size; i++)
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
    for(int i = 0; i < this->_size; i++)
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