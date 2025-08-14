#ifndef FB_GAME_HANDLER_AMQP_WHISPER_H
#define FB_GAME_HANDLER_AMQP_WHISPER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

class whisper : public fb::amqp_handler<fb::game::server, internal_resp::Whisper>
{
public:
    whisper(fb::game::server& server);
    whisper(const whisper&)             = delete;
    whisper(whisper&&)                  = delete;
    whisper& operator= (const whisper&) = delete;
    whisper& operator= (whisper&&)      = delete;

public:
    async::task<void> handle(const internal_resp::Whisper& message) override;
};

} // namespace fb::game::handler::amqp

#endif // FB_GAME_HANDLER_AMQP_WHISPER_H
