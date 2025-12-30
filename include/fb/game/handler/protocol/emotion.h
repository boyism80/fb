#ifndef FB_GAME_HANDLER_EMOTION_H
#define FB_GAME_HANDLER_EMOTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class emotion : public fb::handler::protocol<fb::game::server, game_reqs::emotion>
{
public:
    emotion(fb::game::server& server);
    emotion(const emotion&)             = delete;
    emotion(emotion&&)                  = delete;
    emotion& operator= (const emotion&) = delete;
    emotion& operator= (emotion&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::emotion& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_EMOTION_H
