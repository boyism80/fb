#include <fb/game/shard.h>

using namespace fb::game;

shard_params::shard_params() :
    groups(),
    characters()
{ }

shard::shard()
{
    for (int i = 0; i < SHARD_MODULAR_SIZE; i++)
    {
        this->_data.insert({i, std::make_unique<shard_params>()});
    }
}

shard::~shard()
{ }

uint32_t shard::mod(uint32_t hash) const
{
    return hash % SHARD_MODULAR_SIZE;
}

uint32_t shard::mod(const std::string& name) const
{
    auto hash = std::hash<std::string>{}(name);
    return mod(hash);
}

shard_params* shard::operator[] (uint32_t hash) const
{
    return this->_data.at(this->mod(hash)).get();
}

shard_params* shard::operator[] (const std::string& val) const
{
    return this->_data.at(this->mod(val)).get();
}