#ifndef FB_GAME_HANDLER_AMQP_WRITE_MAIL_H
#define FB_GAME_HANDLER_AMQP_WRITE_MAIL_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class write_mail : public fb::amqp_handler<fb::game::server, internal_resp::WriteMail>
{
public:
    write_mail(fb::game::server& server);
    write_mail(const write_mail&)             = delete;
    write_mail(write_mail&&)                  = delete;
    write_mail& operator= (const write_mail&) = delete;
    write_mail& operator= (write_mail&&)      = delete;

public:
    async::task<void> handle(const internal_resp::WriteMail& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_WRITE_MAIL_H
