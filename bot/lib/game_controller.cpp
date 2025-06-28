#include <fb/bot/game_controller.h>
#include <fb/bot/container.h>

namespace fb::bot {

game_bot_controller::game_bot_controller(bot_container& container) :
    bot_controller<game_bot>(container)
{ }

bool game_bot_controller::decrypt_policy(int cmd) const
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