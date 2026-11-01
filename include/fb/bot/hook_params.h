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
    std::function<void()>                            aborted;
    const void*                                      context_ptr = nullptr;
    std::function<void()>                            interrupted;
    bool                                             interruptible = false;
};

} // namespace fb::bot

#endif