#ifndef FB_GAME_HANDLER_WHISPER_H
#define FB_GAME_HANDLER_WHISPER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class whisper : public fb::handler::protocol<fb::game::server, game_reqs::whisper<V>>
{
public:
    whisper(fb::game::server& server);
    whisper(const whisper&)             = delete;
    whisper(whisper&&)                  = delete;
    whisper& operator= (const whisper&) = delete;
    whisper& operator= (whisper&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::whisper<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_WHISPER_H
