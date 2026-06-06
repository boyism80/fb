#ifndef FB_GAME_HANDLER_AMQP_WRITE_MAILS_H
#define FB_GAME_HANDLER_AMQP_WRITE_MAILS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class write_mails : public fb::handler::amqp<fb::game::server, internal_resp::WriteMails>
{
public:
    write_mails(fb::game::server& server);
    write_mails(const write_mails&)             = delete;
    write_mails(write_mails&&)                  = delete;
    write_mails& operator= (const write_mails&) = delete;
    write_mails& operator= (write_mails&&)      = delete;

public:
    async::task<void> handle(const internal_resp::WriteMails& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_WRITE_MAILS_H
