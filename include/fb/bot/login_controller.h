#ifndef __BOT_LOGIN_CONTROLLER_H__
#define __BOT_LOGIN_CONTROLLER_H__

#include <fb/bot/controller.h>
#include <fb/login/protocol.h>
#include <fb/bot/login_bot.h>

namespace fb::bot {

class login_bot_controller : public bot_controller<login_bot>
{
public:
    using bot_type = login_bot;

protected:
    login_bot_controller(bot_container& container);

public:
    virtual ~login_bot_controller() = default;
    virtual void initialize()       = 0;
    bool         decrypt_policy(int cmd) const override final;

public:
    virtual async::task<void> on_bot_connected(login_bot& bot) override    = 0;
    virtual async::task<void> on_bot_disconnected(login_bot& bot) override = 0;
};

} // namespace fb::bot

#endif