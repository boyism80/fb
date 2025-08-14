#ifndef FB_GAME_HANDLER_EMOTION_H
#define FB_GAME_HANDLER_EMOTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class emotion : public fb::handler<fb::game::server, fb::protocol::game::request::emotion>
{
public:
    emotion(fb::game::server& server);
    emotion(const emotion&)             = delete;
    emotion(emotion&&)                  = delete;
    emotion& operator= (const emotion&) = delete;
    emotion& operator= (emotion&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::emotion& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_EMOTION_H
