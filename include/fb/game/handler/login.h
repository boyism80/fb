#ifndef FB_GAME_HANDLER_LOGIN_H
#define FB_GAME_HANDLER_LOGIN_H

#include <fb/handler.h>
#include <fb/game/server.h>

namespace fb::game::handler {

class login : public fb::handler<fb::game::server, fb::protocol::game::request::login>
{
private:
    using transfer_param = fb_reqs::login::transfer_param;

public:
    login(fb::game::server& server);
    login(const login&)             = delete;
    login(login&&)                  = delete;
    login& operator= (const login&) = delete;
    login& operator= (login&&)      = delete;

private:
    [[nodiscard]] async::task<bool> init_ch(const fb::protocol::internal::Character& response,
                                            fb::game::character&                     ch,
                                            std::optional<uint32_t>                  group,
                                            std::optional<uint32_t>                  clan,
                                            const std::optional<transfer_param>&     transfer);
    void init_option(const fb::protocol::internal::Option& response, fb::game::character& ch);
    void init_items(const std::vector<fb::protocol::internal::Item>& response, fb::game::character& ch);
    void init_spells(const std::vector<fb::protocol::internal::Spell>& response, fb::game::character& ch);
    void init_quests(const std::vector<fb::protocol::internal::Quest>& response, fb::game::character& ch);
    void init_achievements(const std::vector<fb::protocol::internal::Achievement>& response, fb::game::character& ch);
    std::string elapsed_message(const std::string& dt);

public:
    async::task<bool> handle(fb::socket<character>& session, fb::protocol::game::request::login& request) override;
};

} // namespace fb::game::handler

#endif // FB_GAME_HANDLER_LOGIN_H