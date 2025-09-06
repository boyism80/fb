#ifndef FB_GAME_HANDLER_UPDATE_OPTION_H
#define FB_GAME_HANDLER_UPDATE_OPTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class update_option : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::update_option>
{
public:
    update_option(fb::game::server& server);
    update_option(const update_option&)             = delete;
    update_option(update_option&&)                  = delete;
    update_option& operator= (const update_option&) = delete;
    update_option& operator= (update_option&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::update_option& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_UPDATE_OPTION_H
