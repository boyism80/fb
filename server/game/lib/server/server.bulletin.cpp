#include <fb/game/server.h>

using namespace fb::game;

async::task<std::list<bulletin::article>> server::bulletin_list(uint16_t section, uint16_t offset)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    auto&& resp     = co_await this->http.get<internal_resp::GetArticleList>("internal", std::format("/bulletin/{}?offset={}", section, offset));
    auto&  model    = this->model.bulletin[section];
    auto   articles = std::list<bulletin::article>();
    for (auto& summary : resp.summary_list)
    {
        auto dt = fb::model::datetime(summary.created_date);
        articles.push_back(bulletin::article{.id      = summary.id,
                                             .section = model.id,
                                             .user    = summary.user,
                                             .month   = static_cast<uint8_t>(dt.month()),
                                             .day     = static_cast<uint8_t>(dt.day()),
                                             .uname   = summary.user_name,
                                             .title   = summary.title});
    }

    co_return std::move(articles);
}

async::task<bulletin::article> server::read_bulletin(uint16_t section, uint16_t id)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    auto&& resp = co_await this->http.get<internal_resp::GetArticle>("internal", std::format("/bulletin/{}/{}", section, id));
    if (resp.success == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    auto dt = fb::model::datetime(resp.article.created_date);
    co_return bulletin::article{.id       = resp.article.id,
                                .section  = section,
                                .user     = resp.article.user,
                                .month    = static_cast<uint8_t>(dt.month()),
                                .day      = static_cast<uint8_t>(dt.day()),
                                .uname    = resp.article.user_name,
                                .title    = resp.article.title,
                                .contents = resp.article.contents,
                                .next     = resp.next};
}

async::task<void> server::write_bulletin(character& ch, uint16_t section, const std::string& title, const std::string& contents)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    if (ch.condition(this->model.bulletin[section].condition) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));

    if (title.length() > 64)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_TITLE));

    if (contents.length() > 256)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_CONTENTS));

    auto&& resp = co_await this->http.post("internal", "/bulletin/write", WriteArticle{section, ch.id(), title, contents});

    if (resp.success == false)
        throw std::runtime_error("게시글 작성 실패");
}

async::task<void> server::delete_bulletin(character& ch, uint16_t section, uint16_t id)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    if (ch.condition(this->model.bulletin[section].condition) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));

    auto&& resp = co_await this->http.post("internal", "/bulletin/delete", DeleteArticle{id, ch.id()});

    switch (resp.result)
    {
    case -1: // article not found
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    case -2: // article deleted
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    case -3: // no authenticate
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));
    }
}