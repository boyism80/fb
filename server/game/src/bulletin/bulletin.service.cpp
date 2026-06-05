#include <fb/game/bulletin/service.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

using namespace fb::game;
using table             = fb::model::table;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

bulletin_service::bulletin_service(fb::game::server& server) :
    server(server)
{ }

void bulletin_service::assert_section(uint16_t section) const
{
    if (table::bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));
}

void bulletin_service::assert_auth(const character& ch, uint16_t section) const
{
    this->assert_section(section);

    if (ch.condition(table::bulletin[section].condition) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));
}

void bulletin_service::on_delete_error(int32_t result) const
{
    switch (result)
    {
    case -1: // article not found
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    case -2: // article deleted
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    case -3: // no authenticate
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));
    }
}

bulletin::article bulletin_service::to_article(const fb::protocol::internal::ArticleSummary& summary, uint16_t section)
{
    auto dt = fb::model::datetime(summary.created_date);
    return bulletin::article{.id      = summary.id,
                             .section = section,
                             .user    = summary.user,
                             .month   = static_cast<uint8_t>(dt.month()),
                             .day     = static_cast<uint8_t>(dt.day()),
                             .uname   = summary.user_name,
                             .title   = summary.title};
}

bulletin::article bulletin_service::to_article(const fb::protocol::internal::Article& article,
                                               uint16_t                               section,
                                               bool                                   next)
{
    auto dt = fb::model::datetime(article.created_date);
    return bulletin::article{.id       = article.id,
                             .section  = section,
                             .user     = article.user,
                             .month    = static_cast<uint8_t>(dt.month()),
                             .day      = static_cast<uint8_t>(dt.day()),
                             .uname    = article.user_name,
                             .title    = article.title,
                             .contents = article.contents,
                             .next     = next};
}

async::task<std::list<bulletin::article>> bulletin_service::list(uint16_t section, uint16_t offset)
{
    this->assert_section(section);

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.get<internal_resp::GetArticleList>(
        "internal",
        std::format("/bulletin/{}/{}?offset={}", world, section, offset));

    auto articles = std::list<bulletin::article>{};
    for (const auto& summary : resp.summary_list)
    {
        articles.push_back(to_article(summary, section));
    }

    co_return std::move(articles);
}

async::task<bulletin::article> bulletin_service::read(uint16_t section, uint16_t id)
{
    this->assert_section(section);

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.get<internal_resp::GetArticle>(
        "internal",
        std::format("/bulletin/{}/{}/{}", world, section, id));
    if (resp.success == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    co_return to_article(resp.article, section, resp.next);
}

async::task<void>
bulletin_service::write(character& ch, uint16_t section, std::string_view title, std::string_view contents)
{
    this->assert_auth(ch, section);

    if (title.length() > 64)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_TITLE));

    if (contents.length() > 256)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_CONTENTS));

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post(
        "internal",
        "/bulletin/write",
        internal_reqs::WriteArticle{world, section, ch.id, std::string(title), std::string(contents)});

    if (resp.success == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_WRITE_FAILED));
}

async::task<void> bulletin_service::remove(character& ch, uint16_t section, uint16_t id)
{
    this->assert_auth(ch, section);

    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.post("internal",
                                                  "/bulletin/delete",
                                                  internal_reqs::DeleteArticle{world, id, section, ch.id});

    this->on_delete_error(resp.result);
    co_return;
}
