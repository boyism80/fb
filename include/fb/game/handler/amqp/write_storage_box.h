#ifndef FB_GAME_HANDLER_AMQP_WRITE_STORAGE_BOX_H
#define FB_GAME_HANDLER_AMQP_WRITE_STORAGE_BOX_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class write_storage_box : public fb::handler::amqp<fb::game::server, internal_resp::WriteStorageBox>
{
public:
    write_storage_box(fb::game::server& server);
    write_storage_box(const write_storage_box&)             = delete;
    write_storage_box(write_storage_box&&)                  = delete;
    write_storage_box& operator= (const write_storage_box&) = delete;
    write_storage_box& operator= (write_storage_box&&)      = delete;

public:
    async::task<void> handle(const internal_resp::WriteStorageBox& message) override;
};

} // namespace fb::game::handler::amqp

#endif
