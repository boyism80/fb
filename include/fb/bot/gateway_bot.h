#ifndef __BOT_GATEWAY_H__
#define __BOT_GATEWAY_H__

#include <fb/bot/bot.h>
#include <fb/gateway/protocol.h>
#include <shared_mutex>

namespace fb::bot {

class gateway_bot_controller;
template <typename ControllerType> class bot;

class gateway_bot : public bot<gateway_bot>
{
public:
    using bot_controller_type = gateway_bot_controller;

public:
    gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id);
    gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id, const fb::stream& params);
    ~gateway_bot();
};

} // namespace fb::bot

#endif