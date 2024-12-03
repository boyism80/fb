#ifndef __THREAD_PARAMS_H__
#define __THREAD_PARAMS_H__

#include <group.h>
#include <unordered_map>

namespace fb::game {
    
class thread_params
{
public:
    using group_container = std::unordered_map<uint32_t, std::unique_ptr<group>>;

public:
    group_container groups;
};

}

#endif