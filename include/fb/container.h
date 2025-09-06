#ifndef __FB_CONTAINER_H__
#define __FB_CONTAINER_H__

#include <memory>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace fb {

template <typename K, typename V>
class kv_container
{
public:
    using iterator       = std::unordered_map<K, std::shared_ptr<V>>::iterator;
    using const_iterator = std::unordered_map<K, std::shared_ptr<V>>::const_iterator;

private:
    std::vector<std::shared_ptr<V>>           _ptrs;
    std::unordered_map<K, std::shared_ptr<V>> _pairs;

public:
    kv_container()  = default;
    ~kv_container() = default;

public:
    bool contains(const K& k) const
    {
        return this->_pairs.find(k) != this->_pairs.cend();
    }

public:
    std::shared_ptr<V> find(const K& k) const
    {
        auto i = this->_pairs.find(k);
        if (i == this->_pairs.cend())
            return nullptr;

        return i->second;
    }

public:
    void push(K key, std::shared_ptr<V> value)
    {
        this->_ptrs.push_back(value);
        this->_pairs.insert({key, this->_ptrs.back()});
    }

public:
    uint32_t size() const
    {
        return static_cast<uint32_t>(this->_pairs.size());
    }

public:
    std::shared_ptr<V> operator[] (const K& k)
    {
        auto found = this->find(k);
        if (found == nullptr)
        {
            auto sstream = std::stringstream();
            if constexpr (std::is_enum_v<K>)
            {
                sstream << static_cast<int>(k);
            }
            else
            {
                sstream << k;
            }
            sstream << " does not exists";
            throw std::runtime_error(sstream.str());
        }

        return found;
    }

public:
    iterator begin()
    {
        return this->_pairs.begin();
    }

public:
    iterator end()
    {
        return this->_pairs.end();
    }

public:
    const const_iterator begin() const
    {
        return this->_pairs.begin();
    }

public:
    const const_iterator end() const
    {
        return this->_pairs.end();
    }
};

template <typename T>
class array_container : private std::vector<std::unique_ptr<T>>
{
private:
    using super = std::vector<std::unique_ptr<T>>;

public:
    class iterator;
    class const_iterator;

public:
    using super::size;

protected:
    array_container() = default;

public:
    void push(T* value)
    {
        super::push_back(std::unique_ptr<T>(value));
    }

public:
    T* find(uint32_t i) const
    {
        if (i > super::size() - 1)
            return nullptr;

        return &super::at(i);
    }

public:
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
private:
    using super = std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator;

public:
    iterator(const super& i) :
        super(i)
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
private:
    using super = std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator;

public:
    const_iterator(const super& i) :
        super(i)
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