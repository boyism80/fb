#include "module/socket/socket.h"

template <typename T>
fb::session_map<T>::session_map()
{
}

template <typename T>
fb::session_map<T>::~session_map()
{
}

template <typename T>
T* fb::session_map<T>::find(const fb::socket* key)
{
    auto i = __super::find(key);
    if(i == this->end())
        return nullptr;

    return i->second;
}

template <typename T>
T* fb::session_map<T>::find(const fb::socket& key)
{
    return this->find(&key);
}

template <typename T>
T* fb::session_map<T>::push(const fb::socket* key, T* val)
{
    __super::insert(std::make_pair(key, val));
    return val;
}

template <typename T>
void fb::session_map<T>::erase(const fb::socket& key)
{
    __super::erase(&key);
}

template <typename T>
T* fb::session_map<T>::operator[](const fb::socket* key)
{
    return this->find(key);
}

template <typename T>
T* fb::session_map<T>::operator[](const fb::socket& key)
{
    return this->find(key);
}

template <typename T>
void fb::session_container<T>::push(T* session)
{
    this->push_back(session);
}

template <typename T>
void fb::session_container<T>::erase(T* session)
{
    if(session == nullptr)
        return;

    std::vector<T*>::erase(std::find(this->begin(), this->end(), session));
}

template <typename T>
void fb::session_container<T>::erase(uint32_t fd)
{
    this->erase(this->operator[](fd));
}

template <typename T>
inline T* fb::session_container<T>::operator[](uint32_t fd)
{
    auto i = std::find_if
    (
        this->begin(), this->end(), 
        [fd] (T* x) 
        { 
            uint32_t x_fd = static_cast<fb::socket&>(*x).native_handle();
            return x_fd == fd; 
        }
    );
    return i == this->end() ? nullptr : *i;
}