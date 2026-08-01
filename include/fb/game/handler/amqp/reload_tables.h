#ifndef FB_GAME_HANDLER_AMQP_RELOAD_TABLES_H
#define FB_GAME_HANDLER_AMQP_RELOAD_TABLES_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class reload_tables : public fb::handler::amqp<fb::game::server, internal_resp::ReloadTables>
{
public:
    reload_tables(fb::game::server& server);
    reload_tables(const reload_tables&)             = delete;
    reload_tables(reload_tables&&)                  = delete;
    reload_tables& operator= (const reload_tables&) = delete;
    reload_tables& operator= (reload_tables&&)      = delete;

public:
    async::task<void> handle(const internal_resp::ReloadTables& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_RELOAD_TABLES_H
