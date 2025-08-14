#ifndef FB_GAME_HANDLER_ITEM_THROWS_H
#define FB_GAME_HANDLER_ITEM_THROWS_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class item_throws : public fb::handler<fb::game::server, fb::protocol::game::request::item_throws>
{
public:
    item_throws(fb::game::server& server);
    item_throws(const item_throws&)             = delete;
    item_throws(item_throws&&)                  = delete;
    item_throws& operator= (const item_throws&) = delete;
    item_throws& operator= (item_throws&&)      = delete;

    async::task<bool> handle(fb::socket<character>&                    session,
                             fb::protocol::game::request::item_throws& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_THROWS_H
