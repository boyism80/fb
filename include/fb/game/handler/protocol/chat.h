#ifndef FB_GAME_HANDLER_CHAT_H
#define FB_GAME_HANDLER_CHAT_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class chat : public fb::handler::protocol<fb::game::server, game_reqs::chat>
{
public:
    chat(fb::game::server& server);
    chat(const chat&)             = delete;
    chat(chat&&)                  = delete;
    chat& operator= (const chat&) = delete;
    chat& operator= (chat&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::chat& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_CHAT_H
