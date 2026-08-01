#ifndef FB_GAME_HANDLER_AMQP_RELOAD_SCRIPTS_H
#define FB_GAME_HANDLER_AMQP_RELOAD_SCRIPTS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class reload_scripts : public fb::handler::amqp<fb::game::server, internal_resp::ReloadScripts>
{
public:
    reload_scripts(fb::game::server& server);
    reload_scripts(const reload_scripts&)             = delete;
    reload_scripts(reload_scripts&&)                  = delete;
    reload_scripts& operator= (const reload_scripts&) = delete;
    reload_scripts& operator= (reload_scripts&&)      = delete;

public:
    async::task<void> handle(const internal_resp::ReloadScripts& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_RELOAD_SCRIPTS_H
