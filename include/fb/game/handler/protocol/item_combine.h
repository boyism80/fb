#ifndef FB_GAME_HANDLER_ITEM_COMBINE_H
#define FB_GAME_HANDLER_ITEM_COMBINE_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class item_combine : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::item_combine>
{
public:
    item_combine(fb::game::server& server);
    item_combine(const item_combine&)             = delete;
    item_combine(item_combine&&)                  = delete;
    item_combine& operator= (const item_combine&) = delete;
    item_combine& operator= (item_combine&&)      = delete;

    async::task<bool> handle(fb::socket<character>&                     session,
                             fb::protocol::game::request::item_combine& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_COMBINE_H
