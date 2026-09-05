#ifndef FB_GAME_HANDLER_ITEM_DROP_MONEY_H
#define FB_GAME_HANDLER_ITEM_DROP_MONEY_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class item_drop_money : public fb::handler::protocol<fb::game::server, game_reqs::item_drop_money<V>>
{
public:
    item_drop_money(fb::game::server& server);
    item_drop_money(const item_drop_money&)             = delete;
    item_drop_money(item_drop_money&&)                  = delete;
    item_drop_money& operator= (const item_drop_money&) = delete;
    item_drop_money& operator= (item_drop_money&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::item_drop_money<V>& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_ITEM_DROP_MONEY_H
