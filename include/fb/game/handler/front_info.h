#ifndef FB_GAME_HANDLER_FRONT_INFO_H
#define FB_GAME_HANDLER_FRONT_INFO_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class front_info : public fb::handler<fb::game::server, fb::protocol::game::request::front_info>
{
public:
    front_info(fb::game::server& server);
    front_info(const front_info&)             = delete;
    front_info(front_info&&)                  = delete;
    front_info& operator= (const front_info&) = delete;
    front_info& operator= (front_info&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::front_info& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_FRONT_INFO_H
