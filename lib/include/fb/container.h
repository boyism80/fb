#ifndef __FB_CONTAINER_H__
#define __FB_CONTAINER_H__

#include <memory>
#include <unordered_map>
#include <vector>

namespace fb {

template <typename K, typename V>
class kv_container
{
public:
    using iterator       = std::unordered_map<K, V&>::iterator;
    using const_iterator = std::unordered_map<K, V&>::const_iterator;

private:
    std::vector<std::unique_ptr<V>> _ptrs;
    std::unordered_map<K, V&>       _pairs;

public:
    kv_container()  = default;
    ~kv_container() = default;

public:
    bool contains(const K& k) const
    {
        return this->_pairs.find(k) != this->_pairs.cend();
    }

    V* find(const K& k) const
    {
        auto i = this->_pairs.find(k);
        if (i == this->_pairs.cend())
            return nullptr;

        return &i->second;
    }

    void push(K key, V* value)
    {
        this->_ptrs.push_back(std::unique_ptr<V>(value));
        this->_pairs.insert({key, *value});
    }

    uint32_t size() const
    {
        return static_cast<uint32_t>(this->_pairs.size());
    }

    V& operator[] (const K& k)
    {
        auto found = this->find(k);
        if (found == nullptr)
            throw std::runtime_error("does not exists");

        return *found;
    }

    iterator begin()
    {
        return this->_pairs.begin();
    }

    iterator end()
    {
        return this->_pairs.end();
    }

    const const_iterator begin() const
    {
        return this->_pairs.begin();
    }
    const const_iterator end() const
    {
        return this->_pairs.end();
    }
};

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

    T& operator[] (uint32_t i) const
    {
        auto found = this->find(i);
        if (found == nullptr)
            throw std::out_of_range("out of range exception");

        return *found;
    }

public:
    iterator             begin();
    iterator             end();
    const const_iterator begin() const;
    const const_iterator end() const;
};

template <typename T>
class array_container<T>::iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator
{
public:
    iterator(const typename std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator& i) :
        std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator(i)
    { }
    ~iterator() = default;

public:
    T& operator* ()
    {
        return std::vector<std::unique_ptr<T>>::iterator::operator* ();
    }
};

template <typename T>
class array_container<T>::const_iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator
{
public:
    const_iterator(const typename std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator& i) :
        std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator(i)
    { }
    ~const_iterator() = default;

public:
    const T& operator* () const
    {
        return std::vector<std::unique_ptr<T>>::const_iterator::operator* ();
    }
};

} // namespace fb

#endif