#ifndef FB_GAME_HANDLER_AMQP_WHISPER_H
#define FB_GAME_HANDLER_AMQP_WHISPER_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::amqp {

namespace internal_resp = fb::protocol::internal::response;

class whisper : public fb::handler::amqp<fb::game::server, internal_resp::Whisper>
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
