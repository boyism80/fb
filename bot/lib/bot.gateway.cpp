#include <fb/bot/bot.login.h>
#include <fb/bot/bot.gateway.h>
#include <fb/bot/bot.gateway.controller.h>

using namespace fb::bot;

gateway_bot::gateway_bot(bot_controller<gateway_bot>& controller, uint32_t id) :
    bot<gateway_bot>(controller, id)
{ }

gateway_bot::gateway_bot(bot_controller<gateway_bot>& controller, uint32_t id, const fb::stream& params) :
    gateway_bot(controller, id)
{
    // Gateway bot doesn't use transfer parameters, so we ignore them
}

gateway_bot::~gateway_bot()
{ }
