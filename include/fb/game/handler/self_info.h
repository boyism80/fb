#ifndef FB_GAME_HANDLER_SELF_INFO_H
#define FB_GAME_HANDLER_SELF_INFO_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class self_info : public fb::handler<fb::game::server, fb::protocol::game::request::self_info>
{
public:
    self_info(fb::game::server& server);
    self_info(const self_info&)             = delete;
    self_info(self_info&&)                  = delete;
    self_info& operator= (const self_info&) = delete;
    self_info& operator= (self_info&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::self_info& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_SELF_INFO_H
