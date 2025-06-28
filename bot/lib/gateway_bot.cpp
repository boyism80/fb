#include <fb/bot/login_bot.h>
#include <fb/bot/gateway_bot.h>
#include <fb/bot/gateway_controller.h>

using namespace fb::bot;

gateway_bot::gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id) :
    bot<gateway_bot>(bot_controller, id)
{ }

gateway_bot::gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id, const fb::stream& params) :
    gateway_bot(bot_controller, id)
{
    // Gateway bot doesn't use transfer parameters, so we ignore them
}

gateway_bot::~gateway_bot()
{ }
