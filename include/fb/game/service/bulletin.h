#ifndef __FB_GAME_SERVICE_BULLETIN_H__
#define __FB_GAME_SERVICE_BULLETIN_H__

#include <fb/game/bulletin.h>
#include <list>
#include <string>
#include <string_view>
#include <cstdint>

namespace fb::protocol::internal {
class ArticleSummary;
class Article;
} // namespace fb::protocol::internal

namespace fb::game {
class server;
class character;
} // namespace fb::game

namespace fb::game::service {

class bulletin
{
public:
    fb::game::server& server;

public:
    explicit bulletin(fb::game::server& server);

    // clang-format off
    async::task<std::list<fb::game::bulletin::article>> list(uint16_t section, uint16_t offset);
    async::task<fb::game::bulletin::article>            read(uint16_t section, uint16_t id);
    async::task<void>                                   write(character& ch, uint16_t section, std::string_view title, std::string_view contents);
    async::task<void>                                   remove(character& ch, uint16_t section, uint16_t id);
    // clang-format on

private:
    // clang-format off
    static fb::game::bulletin::article to_article(const fb::protocol::internal::ArticleSummary& summary, uint16_t section);
    static fb::game::bulletin::article to_article(const fb::protocol::internal::Article& article, uint16_t section, bool next);
    void assert_section(uint16_t section) const;
    void assert_auth(const character& ch, uint16_t section) const;
    void on_delete_error(int32_t result) const;
    // clang-format on
};

} // namespace fb::game::service

#endif
