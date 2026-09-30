#ifndef __HOOK_PARAMS_H__
#define __HOOK_PARAMS_H__

#include <fb/protocol/header.h>

#include <functional>

namespace fb::bot {

class hook_params
{
public:
    std::function<bool(const fb::protocol::header&)> condition;
    std::function<void(const fb::protocol::header&)> matched;
    const void*                                      context_ptr = nullptr;
};

} // namespace fb::bot

#endif