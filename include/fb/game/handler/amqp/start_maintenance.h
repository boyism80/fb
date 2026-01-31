#ifndef FB_GAME_HANDLER_AMQP_START_MAINTENANCE_H
#define FB_GAME_HANDLER_AMQP_START_MAINTENANCE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class start_maintenance : public fb::handler::amqp<fb::game::server, internal_resp::StartMaintenance>
{
public:
    start_maintenance(fb::game::server& server);
    start_maintenance(const start_maintenance&)             = delete;
    start_maintenance(start_maintenance&&)                  = delete;
    start_maintenance& operator= (const start_maintenance&) = delete;
    start_maintenance& operator= (start_maintenance&&)      = delete;

public:
    async::task<void> handle(const internal_resp::StartMaintenance& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_START_MAINTENANCE_H
