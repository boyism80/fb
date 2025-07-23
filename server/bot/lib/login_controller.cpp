#include <fb/bot/login_controller.h>
#include <fb/bot/container.h>

namespace fb::bot {

login_bot_controller::login_bot_controller(bot_container& container) :
    bot_controller<login_bot>(container)
{ }

bool login_bot_controller::decrypt_policy(int cmd) const
{
    switch (cmd)
    {
    case fb::protocol::response::transfer::header: // Host discovery
        return false;

    default:
        return true;
    }
}

} // namespace fb::bot