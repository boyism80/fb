#ifndef FB_GAME_HANDLER_UPDATE_OPTION_H
#define FB_GAME_HANDLER_UPDATE_OPTION_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class update_option : public fb::handler::protocol<fb::game::server, game_reqs::update_option<V>>
{
public:
    update_option(fb::game::server& server);
    update_option(const update_option&)             = delete;
    update_option(update_option&&)                  = delete;
    update_option& operator= (const update_option&) = delete;
    update_option& operator= (update_option&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::update_option<V>& request) override;
};

template <>
async::task<bool> update_option<fb::protocol::CLIENT_VERSION::v651>::handle(
    fb::socket<character>&                                        session,
    game_reqs::update_option<fb::protocol::CLIENT_VERSION::v651>& request);

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_UPDATE_OPTION_H
