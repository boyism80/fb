#ifndef __HASH_H__
#define __HASH_H__

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>

namespace fb {

template <typename T>
class hash
{
public:
    inline static const uint32_t SHARD_MODULAR_SIZE = 12;

private:
    std::unordered_map<uint32_t, std::unique_ptr<T>> _data;

public:
    hash()
    {
        for (int i = 0; i < SHARD_MODULAR_SIZE; i++)
        {
            this->_data.insert({i, std::make_unique<T>()});
        }
    }

    hash(const hash&) = delete;
    hash(hash&&)      = default;
    ~hash()           = default;

public:
    uint32_t mod(uint32_t k) const
    {
        return k % SHARD_MODULAR_SIZE;
    }

    uint32_t mod(const void* k) const
    {
        return (uint64_t)k % SHARD_MODULAR_SIZE;
    }

    uint32_t mod(const std::string& name) const
    {
        auto k = std::hash<std::string>{}(name);
        return mod(k);
    }

public:
    T* operator[] (uint32_t hash) const
    {
        return this->_data.at(this->mod(hash)).get();
    }

    T* operator[] (const std::string& val) const
    {
        return this->_data.at(this->mod(val)).get();
    }

    T* operator[] (const void* ptr) const
    {
        return this->_data.at(this->mod(ptr)).get();
    }
};

} // namespace fb

#endif