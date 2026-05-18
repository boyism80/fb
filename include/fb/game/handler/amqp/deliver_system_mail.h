#ifndef FB_GAME_HANDLER_AMQP_DELIVER_SYSTEM_MAIL_H
#define FB_GAME_HANDLER_AMQP_DELIVER_SYSTEM_MAIL_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class deliver_system_mail : public fb::handler::amqp<fb::game::server, internal_resp::DeliverSystemMail>
{
public:
    deliver_system_mail(fb::game::server& server);
    deliver_system_mail(const deliver_system_mail&)             = delete;
    deliver_system_mail(deliver_system_mail&&)                  = delete;
    deliver_system_mail& operator= (const deliver_system_mail&) = delete;
    deliver_system_mail& operator= (deliver_system_mail&&)      = delete;

public:
    async::task<void> handle(const internal_resp::DeliverSystemMail& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_DELIVER_SYSTEM_MAIL_H
