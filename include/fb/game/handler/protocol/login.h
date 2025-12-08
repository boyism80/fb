#ifndef FB_GAME_HANDLER_LOGIN_H
#define FB_GAME_HANDLER_LOGIN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler::protocol {

class login : public fb::handler::protocol<fb::game::server, fb::protocol::game::request::login>
{
public:
    login(fb::game::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

private:
    void                                                            init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);
    void                                                            init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);
    void                                                            init_spells(const std::vector<fb::protocol::internal::Spell>& response, fb::game::character& ch);
    void                                                            init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch);
    void                                                            init_achievements(const std::vector<fb::protocol::internal::Achievement>& response, fb::game::character& ch);
    void                                                            init_system_mail(const std::vector<fb::protocol::internal::SystemMailUser>& response, fb::game::character& ch);
    void                                                            init_storage(const fb::protocol::internal::response::Init& response, fb::game::character& ch);
    [[nodiscard]] async::task<std::shared_ptr<fb::game::character>> init(const fb::protocol::game::request::login& request, fb::socket<character>& session);
    std::string                                                     elapsed_message(const std::string& dt);
    [[nodiscard]] async::task<bool>                                 assert_login(const fb::protocol::game::request::login& request);

public:
    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::login& request) override;
};

} // namespace fb::game::handler::protocol

#endif // FB_GAME_HANDLER_LOGIN_H
