#include <fb/bot/container.h>
#include <fb/bot/gateway_controller.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/login_controller.h>

namespace fb::bot {

gateway_bot_controller::gateway_bot_controller(bot_container& container) :
    bot_controller<gateway_bot>(container)
{ }

bool gateway_bot_controller::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case fb::protocol::gateway::response::crypto::header:  // Welcome message
    case fb::protocol::gateway::response::welcome::header: // Crypto exchange
    case fb::protocol::response::transfer::header:         // Host discovery
        return false;

    default:
        return true;
    }
}

} // namespace fb::bot