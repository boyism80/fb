#ifndef __BOT_LOGIN_INTEGRATION_CONTROLLER_H__
#define __BOT_LOGIN_INTEGRATION_CONTROLLER_H__

#include <fb/bot/login_controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/login_bot.h>

namespace fb::bot::integration {

namespace login_resp = fb::protocol::login::response;
namespace login_reqs = fb::protocol::login::request;
namespace fb_resp    = fb::protocol::response;

class login_bot_controller : public fb::bot::login_bot_controller
{
private:
    using super    = fb::bot::login_bot_controller;
    using bot_type = login_bot;

public:
    login_bot_controller(bot_container& container);

    void initialize() override;

public:
    virtual async::task<void> on_bot_connected(login_bot& bot) override;
    virtual async::task<void> on_bot_disconnected(login_bot& bot) override;

private:
    async::task<void> on_agreement(login_bot& bot, const login_resp::terms_agreement& response);
    async::task<void> on_transfer(login_bot& bot, const fb_resp::transfer& response);
};

} // namespace fb::bot::integration

#endif