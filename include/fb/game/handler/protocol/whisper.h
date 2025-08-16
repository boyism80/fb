#ifndef FB_GAME_HANDLER_WHISPER_H
#define FB_GAME_HANDLER_WHISPER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class whisper : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::whisper>
{
public:
    whisper(fb::game::server& server);
    whisper(const whisper&)             = delete;
    whisper(whisper&&)                  = delete;
    whisper& operator= (const whisper&) = delete;
    whisper& operator= (whisper&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::whisper& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_WHISPER_H
