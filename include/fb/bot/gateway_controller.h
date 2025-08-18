#ifndef __BOT_GATEWAY_CONTROLLER_H__
#define __BOT_GATEWAY_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/gateway_bot.h>

namespace fb::bot {

class gateway_bot_controller : public bot_controller<gateway_bot>
{
public:
    using bot_type = gateway_bot;

protected:
    gateway_bot_controller(bot_container& container);

public:
    virtual ~gateway_bot_controller() = default;
    virtual void initialize()         = 0;
    bool         decrypt_policy(int cmd) const override final;

public:
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override    = 0;
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override = 0;
};

} // namespace fb::bot

#endif