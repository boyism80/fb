#ifndef __HOOK_PARAMS_H__
#define __HOOK_PARAMS_H__

#include <functional>
#include <fb/protocol/header.h>

namespace fb::bot {

class hook_params
{
public:
    std::function<bool(const fb::protocol::header&)> condition;
    std::function<void(const fb::protocol::header&)> matched;
};

} // namespace fb::bot

#endif