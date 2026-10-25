#ifndef FB_GAME_HANDLER_AMQP_WRITE_SYSTEM_STORAGE_BOX_H
#define FB_GAME_HANDLER_AMQP_WRITE_SYSTEM_STORAGE_BOX_H

#include <fb/game/server.h>
#include <fb/handler.h>

namespace fb::game::handler::amqp {

class write_system_storage_box : public fb::handler::amqp<fb::game::server, internal_resp::WriteSystemStorageBox>
{
public:
    write_system_storage_box(fb::game::server& server);
    write_system_storage_box(const write_system_storage_box&)             = delete;
    write_system_storage_box(write_system_storage_box&&)                  = delete;
    write_system_storage_box& operator= (const write_system_storage_box&) = delete;
    write_system_storage_box& operator= (write_system_storage_box&&)      = delete;

public:
    async::task<void> handle(const internal_resp::WriteSystemStorageBox& message) override;
};

} // namespace fb::game::handler::amqp

#endif
