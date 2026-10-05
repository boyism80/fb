#include <fb/bot/gateway_bot.h>

#include <fb/bot/gateway_controller.h>
#include <fb/bot/login_bot.h>

#include <cstdint>
#include <optional>

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

const std::optional<fb::bot::credential>& gateway_bot::credential() const
{
    return this->_credential;
}

void gateway_bot::credential(const std::optional<fb::bot::credential>& value)
{
    this->_credential = value;
}

uint32_t gateway_bot::reconnect_from() const
{
    return this->_reconnect_from;
}

void gateway_bot::reconnect_from(uint32_t value)
{
    this->_reconnect_from = value;
}
