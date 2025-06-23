#ifndef __FB_CONTAINER_H__
#define __FB_CONTAINER_H__

#include <memory>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace fb {

/**
 * @brief      A key-value container that manages unique pointers and provides fast lookup.
 *
 * @tparam     K     The key type.
 * @tparam     V     The value type.
 */
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
    /**
     * @brief      Checks if the container contains the specified key.
     *
     * @param[in]  k     The key to check.
     *
     * @return     True if the key exists, false otherwise.
     */
    bool contains(const K& k) const
    {
        return this->_pairs.find(k) != this->_pairs.cend();
    }

public:
    /**
     * @brief      Finds the value associated with the specified key.
     *
     * @param[in]  k     The key to search for.
     *
     * @return     A pointer to the value if found, nullptr otherwise.
     */
    std::shared_ptr<V> find(const K& k) const
    {
        auto i = this->_pairs.find(k);
        if (i == this->_pairs.cend())
            return nullptr;

        return i->second;
    }

public:
    /**
     * @brief      Adds a key-value pair to the container.
     *
     * @param[in]  key    The key to associate with the value.
     * @param      value  The value to store (ownership is transferred).
     */
    void push(K key, std::shared_ptr<V> value)
    {
        this->_ptrs.push_back(value);
        this->_pairs.insert({key, this->_ptrs.back()});
    }

public:
    /**
     * @brief      Gets the number of elements in the container.
     *
     * @return     The size of the container.
     */
    uint32_t size() const
    {
        return static_cast<uint32_t>(this->_pairs.size());
    }

public:
    /**
     * @brief      Accesses the value associated with the specified key.
     *
     * @param[in]  k     The key to look up.
     *
     * @return     A reference to the value.
     *
     * @throws     std::runtime_error if the key does not exist.
     */
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
    /**
     * @brief      Gets an iterator to the beginning of the container.
     *
     * @return     An iterator to the first element.
     */
    iterator begin()
    {
        return this->_pairs.begin();
    }

public:
    /**
     * @brief      Gets an iterator to the end of the container.
     *
     * @return     An iterator to one past the last element.
     */
    iterator end()
    {
        return this->_pairs.end();
    }

public:
    /**
     * @brief      Gets a const iterator to the beginning of the container.
     *
     * @return     A const iterator to the first element.
     */
    const const_iterator begin() const
    {
        return this->_pairs.begin();
    }

public:
    /**
     * @brief      Gets a const iterator to the end of the container.
     *
     * @return     A const iterator to one past the last element.
     */
    const const_iterator end() const
    {
        return this->_pairs.end();
    }
};

/**
 * @brief      An array container that manages unique pointers with indexed access.
 *
 * @tparam     T     The element type.
 */
template <typename T>
class array_container : private std::vector<std::unique_ptr<T>>
{
private:
    using super = std::vector<std::unique_ptr<T>>;

public:
    /**
     * @brief      Iterator class for array_container.
     */
    class iterator;

    /**
     * @brief      Const iterator class for array_container.
     */
    class const_iterator;

public:
    using super::size;

protected:
    array_container() = default;

public:
    /**
     * @brief      Adds an element to the container.
     *
     * @param      value  The value to add (ownership is transferred).
     */
    void push(T* value)
    {
        super::push_back(std::unique_ptr<T>(value));
    }

public:
    /**
     * @brief      Finds the element at the specified index.
     *
     * @param[in]  i     The index to search for.
     *
     * @return     A pointer to the element if found, nullptr otherwise.
     */
    T* find(uint32_t i) const
    {
        if (i > super::size() - 1)
            return nullptr;

        return &super::at(i);
    }

public:
    /**
     * @brief      Accesses the element at the specified index.
     *
     * @param[in]  i     The index to access.
     *
     * @return     A reference to the element.
     *
     * @throws     std::out_of_range if the index is out of bounds.
     */
    T& operator[] (uint32_t i) const
    {
        auto found = this->find(i);
        if (found == nullptr)
            throw std::out_of_range("out of range exception");

        return *found;
    }

public:
    /**
     * @brief      Gets an iterator to the beginning of the container.
     *
     * @return     An iterator to the first element.
     */
    iterator begin();

    /**
     * @brief      Gets an iterator to the end of the container.
     *
     * @return     An iterator to one past the last element.
     */
    iterator end();

    /**
     * @brief      Gets a const iterator to the beginning of the container.
     *
     * @return     A const iterator to the first element.
     */
    const const_iterator begin() const;

    /**
     * @brief      Gets a const iterator to the end of the container.
     *
     * @return     A const iterator to one past the last element.
     */
    const const_iterator end() const;
};

/**
 * @brief      Iterator class for array_container.
 *
 * @tparam     T     The element type.
 */
template <typename T>
class array_container<T>::iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator
{
private:
    using super = std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator;

public:
    /**
     * @brief      Constructs an iterator from a base iterator.
     *
     * @param[in]  i     The base iterator.
     */
    iterator(const super& i) :
        super(i)
    { }
    ~iterator() = default;

public:
    /**
     * @brief      Dereferences the iterator to access the element.
     *
     * @return     A reference to the element.
     */
    T& operator* ()
    {
        return std::vector<std::unique_ptr<T>>::iterator::operator* ();
    }
};

/**
 * @brief      Const iterator class for array_container.
 *
 * @tparam     T     The element type.
 */
template <typename T>
class array_container<T>::const_iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator
{
private:
    using super = std::vector<std::unique_ptr<std::unique_ptr<T>>>::const_iterator;

public:
    /**
     * @brief      Constructs a const iterator from a base iterator.
     *
     * @param[in]  i     The base iterator.
     */
    const_iterator(const super& i) :
        super(i)
    { }
    ~const_iterator() = default;

public:
    /**
     * @brief      Dereferences the iterator to access the element.
     *
     * @return     A const reference to the element.
     */
    const T& operator* () const
    {
        return std::vector<std::unique_ptr<T>>::const_iterator::operator* ();
    }
};

} // namespace fb

#endif