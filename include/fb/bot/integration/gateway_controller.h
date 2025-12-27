#ifndef __BOT_GATEWAY_INTEGRATION_CONTROLLER_H__
#define __BOT_GATEWAY_INTEGRATION_CONTROLLER_H__

#include <fb/bot/gateway_controller.h>
#include <fb/gateway/protocol.h>
#include <fb/bot/gateway_bot.h>

namespace fb::bot::integration {

namespace gateway_resp = fb::protocol::gateway::response;
namespace fb_resp      = fb::protocol::response;

class gateway_bot_controller : public fb::bot::gateway_bot_controller
{
private:
    using super    = fb::bot::gateway_bot_controller;
    using bot_type = gateway_bot;

public:
    gateway_bot_controller(bot_container& container);

private:
    void              initialize() override;
    async::task<void> on_welcome(gateway_bot& bot, const gateway_resp::welcome& response);
    async::task<void> on_crt(gateway_bot& bot, const gateway_resp::encryption& response);
    async::task<void> on_hosts(gateway_bot& bot, const gateway_resp::endpoint& response);
    async::task<void> on_transfer(gateway_bot& bot, const fb_resp::transfer& response);

public:
    virtual async::task<void> on_bot_connected(gateway_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(gateway_bot& bot) override;
};

} // namespace fb::bot::integration

#endif