#ifndef __HASH_H__
#define __HASH_H__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>

namespace fb {

/**
 * @brief      A sharded hash container that distributes objects across multiple shards.
 *
 *             This template class implements a sharded hash table where objects are
 *             distributed across a fixed number of shards (buckets) to improve
 *             performance and reduce contention in multi-threaded environments.
 *             Each shard contains a unique_ptr to an object of type T.
 *
 * @tparam     T     The type of objects stored in each shard.
 */
template <typename T>
class hash
{
public:
    /**
     * @brief      The number of shards used for distributing objects.
     */
    inline static const uint32_t SHARD_MODULAR_SIZE = 12;

private:
    std::unordered_map<uint32_t, std::unique_ptr<T>> _data;

public:
    /**
     * @brief      Constructs a sharded hash container.
     *
     *             Initializes all shards by creating a unique_ptr<T> for each shard.
     *             Each shard is identified by an index from 0 to SHARD_MODULAR_SIZE-1.
     */
    hash()
    {
        for (int i = 0; i < SHARD_MODULAR_SIZE; i++)
        {
            this->_data.insert({i, std::make_unique<T>()});
        }
    }

    /**
     * @brief      Copy constructor is deleted to prevent copying.
     */
    hash(const hash&) = delete;

    /**
     * @brief      Move constructor is defaulted.
     */
    hash(hash&&) = default;

    /**
     * @brief      Destructor is defaulted.
     */
    ~hash() = default;

public:
    /**
     * @brief      Computes the shard index for a 32-bit integer key.
     *
     * @param[in]  k     The integer key to hash.
     *
     * @return     The shard index (0 to SHARD_MODULAR_SIZE-1).
     */
    uint32_t mod(uint32_t k) const
    {
        return k % SHARD_MODULAR_SIZE;
    }

    /**
     * @brief      Computes the shard index for a pointer key.
     *
     * @param[in]  k     The pointer to hash.
     *
     * @return     The shard index (0 to SHARD_MODULAR_SIZE-1).
     */
    uint32_t mod(const void* k) const
    {
        return (uint64_t)k % SHARD_MODULAR_SIZE;
    }

    /**
     * @brief      Computes the shard index for a string key.
     *
     * @param[in]  name  The string to hash.
     *
     * @return     The shard index (0 to SHARD_MODULAR_SIZE-1).
     */
    uint32_t mod(const std::string& name) const
    {
        auto k = std::hash<std::string>{}(name);
        return mod(k);
    }

public:
    /**
     * @brief      Gets the object from the shard corresponding to the hash value.
     *
     * @param[in]  hash  The hash value to determine the shard.
     *
     * @return     A pointer to the object in the corresponding shard.
     */
    T* operator[] (uint32_t hash) const
    {
        return this->_data.at(this->mod(hash)).get();
    }

    /**
     * @brief      Gets the object from the shard corresponding to the string key.
     *
     * @param[in]  val   The string key to hash and determine the shard.
     *
     * @return     A pointer to the object in the corresponding shard.
     */
    T* operator[] (const std::string& val) const
    {
        return this->_data.at(this->mod(val)).get();
    }

    /**
     * @brief      Gets the object from the shard corresponding to the pointer key.
     *
     * @param[in]  ptr   The pointer to hash and determine the shard.
     *
     * @return     A pointer to the object in the corresponding shard.
     */
    T* operator[] (const void* ptr) const
    {
        return this->_data.at(this->mod(ptr)).get();
    }
};

} // namespace fb

#endif