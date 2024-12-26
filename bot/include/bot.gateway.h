#ifndef __BOT_GATEWAY_H__
#define __BOT_GATEWAY_H__

#include <bot.h>
#include <fb/protocol/gateway.h>
#include <shared_mutex>

namespace fb::bot {

class gateway_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;
    inline static uint32_t          _count = 0;

public:
    gateway_bot(bot_container& owner, uint32_t id);
    ~gateway_bot();

protected:
    async::task<void> on_connected() override final;
    async::task<void> on_disconnected() override final;

private:
    async::task<void> handle_welcome(const fb::protocol::gateway::response::welcome& response);
    async::task<void> handle_crt(const fb::protocol::gateway::response::crt& response);
    async::task<void> handle_hosts(const fb::protocol::gateway::response::hosts& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);

protected:
    bool decrypt_policy(int cmd) const;
};

} // namespace fb::bot

#endif