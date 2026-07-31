#ifndef FB_LOGIN_HANDLER_AMQP_RELOAD_TABLES_H
#define FB_LOGIN_HANDLER_AMQP_RELOAD_TABLES_H

#include <fb/handler.h>
#include <fb/login/server.h>

namespace fb::login::handler::amqp {

class reload_tables : public fb::handler::amqp<fb::login::server, internal_resp::ReloadTables>
{
public:
    reload_tables(fb::login::server& server);
    reload_tables(const reload_tables&)             = delete;
    reload_tables(reload_tables&&)                  = delete;
    reload_tables& operator= (const reload_tables&) = delete;
    reload_tables& operator= (reload_tables&&)      = delete;

public:
    async::task<void> handle(const internal_resp::ReloadTables& message) override;
};

} // namespace fb::login::handler::amqp

#endif // FB_LOGIN_HANDLER_AMQP_RELOAD_TABLES_H
