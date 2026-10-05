#ifndef __BOT_GATEWAY_H__
#define __BOT_GATEWAY_H__

#include <fb/bot/bot.h>
#include <fb/gateway/protocol.h>

#include <cstdint>
#include <optional>

namespace fb::bot {

class gateway_bot_controller;
template <typename ControllerType> class bot;

class gateway_bot : public bot<gateway_bot>
{
private:
    std::optional<fb::bot::credential> _credential;
    uint32_t                           _reconnect_from = 0;

public:
    using bot_controller_type = gateway_bot_controller;

public:
    gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id);
    gateway_bot(bot_controller<gateway_bot>& bot_controller, uint32_t id, const fb::stream& params);
    ~gateway_bot();

public:
    const std::optional<fb::bot::credential>& credential() const;
    void                                      credential(const std::optional<fb::bot::credential>& value);
    uint32_t                                  reconnect_from() const;
    void                                      reconnect_from(uint32_t value);
};

} // namespace fb::bot

#endif