#ifndef FB_GAME_HANDLER_BULLETIN_H
#define FB_GAME_HANDLER_BULLETIN_H

#include <fb/handler.h>
#include <fb/game/server.h>
#include <memory>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

template <fb::protocol::CLIENT_VERSION V>
class bulletin : public fb::handler::protocol<fb::game::server, game_reqs::bulletin<V>>
{
public:
    bulletin(fb::game::server& server);
    bulletin(const bulletin&)             = delete;
    bulletin(bulletin&&)                  = delete;
    bulletin& operator= (const bulletin&) = delete;
    bulletin& operator= (bulletin&&)      = delete;

    async::task<bool> handle(fb::socket<character>& session, game_reqs::bulletin<V>& request) override;

private:
    async::task<void> handle_sections(character* ch);
    async::task<void> handle_articles(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
    async::task<void> handle_article(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
    async::task<void> handle_write(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
    async::task<void> handle_delete(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
    async::task<void> handle_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
    async::task<void> handle_send_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin<V>& request);
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_BULLETIN_H
