#ifndef FB_GAME_HANDLER_LOGIN_H
#define FB_GAME_HANDLER_LOGIN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

namespace game_reqs = fb::protocol::game::request;

class login : public fb::handler::protocol<fb::game::server, game_reqs::login>
{
    using ch_ptr_t = std::shared_ptr<character>;

public:
    login(fb::game::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

private:
    void                  init_option(const internal::Option& response, character& ch);
    void                  init_items(const std::vector<internal::Item>& response, character& ch);
    void                  init_spells(const std::vector<internal::Spell>& response, character& ch);
    void                  init_quests(const std::vector<internal::Quest>& response, character& ch);
    void                  init_marketplace(const std::vector<internal::MarketplacePending>& response, character& ch);
    void                  init_achievements(const std::vector<internal::Achievement>& response, character& ch);
    void                  init_storage(const internal_resp::Init& response, character& ch);
    async::task<ch_ptr_t> init(const game_reqs::login& request, fb::socket<character>& session);
    std::string           elapsed_message(std::string_view dt);
    async::task<bool>     assert_login(const game_reqs::login& request);

public:
    async::task<bool> handle(fb::socket<character>& session, game_reqs::login& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_LOGIN_H
