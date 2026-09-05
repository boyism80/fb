#ifndef FB_GAME_HANDLER_CHAT_H
#define FB_GAME_HANDLER_CHAT_H

#include <fb/handler.h>
#include <fb/game/server.h>
#include <memory>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class chat : public fb::handler::protocol<fb::game::server, game_reqs::chat<V>>
{
public:
    chat(fb::game::server& server);
    chat(const chat&)             = delete;
    chat(chat&&)                  = delete;
    chat& operator= (const chat&) = delete;
    chat& operator= (chat&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::chat<V>& request) override;

private:
    // clang-format off
    async::task<bool> try_command(character* ch, std::weak_ptr<character> weak, game_reqs::chat<V>& request);
    void              handle_normal_chat(character* ch, game_reqs::chat<V>& request, const std::shared_ptr<fb::game::map>& map);
    // clang-format on
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_CHAT_H
