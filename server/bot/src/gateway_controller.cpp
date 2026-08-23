#include <fb/bot/gateway_controller.h>
#include <fb/bot/container.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/login_bot.h>
#include <fb/bot/login_controller.h>

namespace fb::bot {

gateway_bot_controller::gateway_bot_controller(bot_container& container) :
    bot_controller<gateway_bot>(container)
{ }

} // namespace fb::bot
