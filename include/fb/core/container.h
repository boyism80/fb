#ifndef	__FB_CONTAINER_H__
#define	__FB_CONTAINER_H__

#include <memory>
#include <unordered_map>
#include <vector>

namespace fb {

template <typename K, typename V>
class kv_container : private std::unordered_map<K, std::unique_ptr<V>>
{
public:
    class iterator;
    class const_iterator;

public:
    using std::unordered_map<K,std::unique_ptr<V>>::size;

protected:
    kv_container() = default;

public:
    void push(K key, V* value)
    {
        std::unordered_map<K, std::unique_ptr<V>>::insert({ key, std::unique_ptr<V>(value) });
    }

    bool contains(K key) const
    {
        auto i = std::unordered_map<K, std::unique_ptr<V>>::find(key);
        return (i != std::unordered_map<K, std::unique_ptr<V>>::cend());
    }

	V* find(const K& key) const
    {
        auto i = std::unordered_map<K, std::unique_ptr<V>>::find(key);
        if(i == std::unordered_map<K, std::unique_ptr<V>>::cend())
            return nullptr;

        return i->second.get();
    }

    V& operator [] (const K& key) const 
    {
        auto found = this->find(key);
        if(found == nullptr)
            throw std::runtime_error("does not exists");

        return *found;
    }

public:
    iterator begin();
    iterator end();
    const const_iterator begin() const;
    const const_iterator end() const;
};

template <typename K, typename V>
class kv_container<K,V>::iterator : public std::unordered_map<K, std::unique_ptr<V>>::iterator
{
private:
    std::optional<std::pair<K, V&>> _pair;

public:
    iterator(const typename std::unordered_map<K, std::unique_ptr<V>>::iterator& i, const std::unordered_map<K, std::unique_ptr<V>>* container) : std::unordered_map<K, std::unique_ptr<V>>::iterator(i),
        _pair(i != container->end() ? std::make_optional<std::pair<K, V&>>(i->first, *i->second) : std::nullopt)
    { }
    ~iterator() = default;

public:
    std::pair<K,V&>& operator * ()
    {
        return this->_pair.value();
    }
};

template <typename K, typename V>
class kv_container<K,V>::const_iterator : public std::unordered_map<K, std::unique_ptr<V>>::const_iterator
{
private:
    const std::optional<std::pair<K, V&>> _pair;

public:
    const_iterator(const typename std::unordered_map<K, std::unique_ptr<V>>::const_iterator& i, const std::unordered_map<K, std::unique_ptr<V>>* container) : std::unordered_map<K, std::unique_ptr<V>>::const_iterator(i),
        _pair(i != container->end() ? std::make_optional<std::pair<K, V&>>(i->first, *i->second) : std::nullopt)
    { }
    ~const_iterator() = default;

public:
    const std::pair<K,V&>& operator * () const
    {
        return this->_pair.value();
    }
};

template <typename K, typename V>
kv_container<K,V>::iterator kv_container<K,V>::begin()
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::begin(), this);
}

template <typename K, typename V>
kv_container<K,V>::iterator kv_container<K,V>::end()
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::end(), this);
}

template <typename K, typename V>
const typename kv_container<K,V>::const_iterator kv_container<K,V>::begin() const
{
    return kv_container<K,V>::const_iterator(std::unordered_map<K, std::unique_ptr<V>>::begin(), this);
}

template <typename K, typename V>
const typename kv_container<K,V>::const_iterator kv_container<K,V>::end() const
{
    return kv_container<K,V>::const_iterator(std::unordered_map<K, std::unique_ptr<V>>::end(), this);
}

template <typename T>
class array_container : private std::vector<std::unique_ptr<T>>
{
public:
    class iterator;
    class const_iterator;

public:
    using std::vector<std::unique_ptr<T>>::size;

protected:
    array_container() = default;

public:
    void push(T* value)
    {
        std::vector<std::unique_ptr<T>>::push_back(std::unique_ptr<T>(value));
    }

    T* find(uint32_t i) const
    {
        if (i > std::vector<std::unique_ptr<T>>::size() - 1)
            return nullptr;

        return &std::vector<std::unique_ptr<T>>::at(i);
    }

    T& operator [] (uint32_t i) const
    {
        auto found = this->find(i);
        if (found == nullptr)
            throw std::out_of_range("out of range exception");

        return *found;
    }

public:
    iterator begin();
    iterator end();
    const const_iterator begin() const;
    const const_iterator end() const;
};

template <typename T>
class array_container<T>::iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator
{
public:
    iterator(const typename std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator& i) : std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator(i)
    {}
    ~iterator() = default;

public:
    T& operator * ()
    {
        return std::vector<std::unique_ptr<T>>::iterator::operator*();
    }
};

template <typename T>
class array_container<T>::const_iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator
{
public:
    const_iterator(const typename std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator& i) : std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator(i)
    {}
    ~const_iterator() = default;

public:
    const T& operator * () const
    {
        return std::vector<std::unique_ptr<T>>::const_iterator::operator*();
    }
};

}

#endif