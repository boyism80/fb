#ifndef FB_GAME_HANDLER_AMQP_SEND_SYSTEM_MAIL_H
#define FB_GAME_HANDLER_AMQP_SEND_SYSTEM_MAIL_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::amqp {

class send_system_mail : public fb::handler::amqp<fb::game::server, internal_resp::SendSystemMail>
{
public:
    send_system_mail(fb::game::server& server);
    send_system_mail(const send_system_mail&)             = delete;
    send_system_mail(send_system_mail&&)                  = delete;
    send_system_mail& operator= (const send_system_mail&) = delete;
    send_system_mail& operator= (send_system_mail&&)      = delete;

public:
    async::task<void> handle(const internal_resp::SendSystemMail& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_SEND_SYSTEM_MAIL_H
