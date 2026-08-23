#include <fb/bot/login_controller.h>
#include <fb/bot/container.h>

namespace fb::bot {

login_bot_controller::login_bot_controller(bot_container& container) :
    bot_controller<login_bot>(container)
{ }

} // namespace fb::bot
