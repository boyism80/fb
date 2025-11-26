#ifndef FB_GAME_HANDLER_AMQP_STORAGE_PENDING_PERSONAL_H
#define FB_GAME_HANDLER_AMQP_STORAGE_PENDING_PERSONAL_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class storage_pending_personal : public fb::handler::amqp<fb::game::server, internal_resp::GetStoragePending>
{
public:
    storage_pending_personal(fb::game::server& server);
    storage_pending_personal(const storage_pending_personal&)             = delete;
    storage_pending_personal(storage_pending_personal&&)                  = delete;
    storage_pending_personal& operator= (const storage_pending_personal&) = delete;
    storage_pending_personal& operator= (storage_pending_personal&&)      = delete;

public:
    async::task<void> handle(const internal_resp::GetStoragePending& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_STORAGE_PENDING_PERSONAL_H
