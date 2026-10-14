#ifndef FB_GAME_HANDLER_AMQP_DISTRIBUTE_SYSTEM_STORAGE_H
#define FB_GAME_HANDLER_AMQP_DISTRIBUTE_SYSTEM_STORAGE_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::amqp {

class distribute_system_storage : public fb::handler::amqp<fb::game::server, internal_resp::DistributeSystemStorage>
{
public:
    distribute_system_storage(fb::game::server& server);
    distribute_system_storage(const distribute_system_storage&)             = delete;
    distribute_system_storage(distribute_system_storage&&)                  = delete;
    distribute_system_storage& operator= (const distribute_system_storage&) = delete;
    distribute_system_storage& operator= (distribute_system_storage&&)      = delete;

public:
    async::task<void> handle(const internal_resp::DistributeSystemStorage& message) override;
};

} // namespace fb::game::handler::amqp

#endif
