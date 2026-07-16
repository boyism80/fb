#ifndef FB_GAME_HANDLER_AMQP_DELIVER_SYSTEM_STORAGE_H
#define FB_GAME_HANDLER_AMQP_DELIVER_SYSTEM_STORAGE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class deliver_system_storage : public fb::handler::amqp<fb::game::server, internal_resp::DeliverSystemStorage>
{
public:
    deliver_system_storage(fb::game::server& server);
    deliver_system_storage(const deliver_system_storage&)             = delete;
    deliver_system_storage(deliver_system_storage&&)                  = delete;
    deliver_system_storage& operator= (const deliver_system_storage&) = delete;
    deliver_system_storage& operator= (deliver_system_storage&&)      = delete;

public:
    async::task<void> handle(const internal_resp::DeliverSystemStorage& message) override;
};

} // namespace fb::game::handler::amqp

#endif
