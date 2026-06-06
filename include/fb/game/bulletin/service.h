#ifndef __BULLETIN_SERVICE_H__
#define __BULLETIN_SERVICE_H__

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

class bulletin_service
{
public:
    fb::game::server& server;

public:
    explicit bulletin_service(fb::game::server& server);

    async::task<std::list<bulletin::article>> list(uint16_t section, uint16_t offset);
    async::task<bulletin::article>            read(uint16_t section, uint16_t id);
    async::task<void> write(character& ch, uint16_t section, std::string_view title, std::string_view contents);
    async::task<void> remove(character& ch, uint16_t section, uint16_t id);

private:
    static bulletin::article to_article(const fb::protocol::internal::ArticleSummary& summary, uint16_t section);
    static bulletin::article to_article(const fb::protocol::internal::Article& article, uint16_t section, bool next);

    void assert_section(uint16_t section) const;
    void assert_auth(const character& ch, uint16_t section) const;
    void on_delete_error(int32_t result) const;
};

} // namespace fb::game

#endif
