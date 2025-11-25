#ifndef FB_GAME_HANDLER_AMQP_WRITE_SYSTEM_MAIL_H
#define FB_GAME_HANDLER_AMQP_WRITE_SYSTEM_MAIL_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class write_system_mail : public fb::handler::amqp<fb::game::server, internal_resp::WriteSystemMail>
{
public:
    write_system_mail(fb::game::server& server);
    write_system_mail(const write_system_mail&)             = delete;
    write_system_mail(write_system_mail&&)                  = delete;
    write_system_mail& operator= (const write_system_mail&) = delete;
    write_system_mail& operator= (write_system_mail&&)      = delete;

public:
    async::task<void> handle(const internal_resp::WriteSystemMail& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_WRITE_SYSTEM_MAIL_H
