#ifndef __HOOK_PARAMS_H__
#define __HOOK_PARAMS_H__

#include <functional>
#include <fb/protocol/protocol.h>

namespace fb::bot {

class hook_params
{
public:
    std::function<bool(const fb::protocol::base::header&)> condition;
    std::function<void(const fb::protocol::base::header&)> matched;
};

} // namespace fb::bot

#endif