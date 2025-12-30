#ifndef FB_GAME_HANDLER_AMQP_STORAGE_PENDING_FETCH_H
#define FB_GAME_HANDLER_AMQP_STORAGE_PENDING_FETCH_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class storage_pending_fetch : public fb::handler::amqp<fb::game::server, internal_resp::GetStoragePending>
{
public:
    storage_pending_fetch(fb::game::server& server);
    storage_pending_fetch(const storage_pending_fetch&)             = delete;
    storage_pending_fetch(storage_pending_fetch&&)                  = delete;
    storage_pending_fetch& operator= (const storage_pending_fetch&) = delete;
    storage_pending_fetch& operator= (storage_pending_fetch&&)      = delete;

public:
    async::task<void> handle(const internal_resp::GetStoragePending& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_STORAGE_PENDING_FETCH_H
