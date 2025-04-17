#ifndef __BOT_LOGIN_H__
#define __BOT_LOGIN_H__

#include <fb/bot/bot.h>
#include <fb/login/protocol.h>
#include <random.h>
#include <fb/encoding.h>
#include <shared_mutex>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace fb::bot {

class login_bot : public base_bot
{
public:
    inline static std::shared_mutex _mutex;
    inline static uint32_t          _count = 0;

public:
    login_bot(bot_container& owner, uint32_t id);
    login_bot(bot_container& owner, uint32_t id, const fb::stream& params);
    ~login_bot();

private:
    std::string generate_id() const;

protected:
    async::task<void> on_connected() override final;
    async::task<void> on_disconnected() override final;
    bool              decrypt_policy(int cmd) const;

public:
    async::task<void> handle_agreement(const fb::protocol::login::response::agreement& response);
    async::task<void> handle_transfer(const fb::protocol::response::transfer& response);
};

} // namespace fb::bot

#endif