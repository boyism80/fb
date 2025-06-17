#include <fb/game/context.h>

using namespace fb::game;

async::task<std::list<bulletin::article>> context::bulletin_list(uint16_t section, uint16_t offset)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    auto&& resp =
        co_await this->get<internal_resp::GetArticleList>("internal",
                                                          std::format("/bulletin/{}?offset={}", section, offset));

    auto& model    = this->model.bulletin[section];
    auto  articles = std::list<bulletin::article>();
    for (auto& summary : resp.summary_list)
    {
        auto dt = fb::model::datetime(summary.created_date);
        articles.push_back(bulletin::article{summary.id,
                                             model.id,
                                             summary.user,
                                             summary.user_name,
                                             summary.title,
                                             static_cast<uint8_t>(dt.month()),
                                             static_cast<uint8_t>(dt.day())});
    }

    co_return std::move(articles);
}

async::task<bulletin::article> context::read_bulletin(uint16_t section, uint16_t id)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    auto&& resp =
        co_await this->get<internal_resp::GetArticle>("internal", std::format("/bulletin/{}/{}", section, id));
    if (resp.success == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_ARTICLE_NOT_EXIST));

    auto dt = fb::model::datetime(resp.article.created_date);
    co_return bulletin::article{resp.article.id,
                                section,
                                resp.article.user,
                                resp.article.user_name,
                                resp.article.title,
                                static_cast<uint8_t>(dt.month()),
                                static_cast<uint8_t>(dt.day()),
                                resp.article.contents,
                                resp.next};
}

async::task<void>
context::write_bulletin(character& ch, uint16_t section, const std::string& title, const std::string& contents)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    if (ch.condition(this->model.bulletin[section].condition) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));

    if (title.length() > 64)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_TITLE));

    if (contents.length() > 256)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_TOO_LONG_CONTENTS));

    auto&& resp = co_await this->post<internal_reqs::WriteArticle, internal_resp::WriteArticle>(
        "internal",
        "/bulletin/write",
        internal_reqs::WriteArticle{section, ch.id(), title, contents});

    if (resp.success == false)
        throw std::runtime_error("게시글 작성 실패");
}

async::task<void> context::delete_bulletin(character& ch, uint16_t section, uint16_t id)
{
    if (this->model.bulletin.contains(section) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_SECTION_NOT_EXIST));

    if (ch.condition(this->model.bulletin[section].condition) == false)
        throw std::runtime_error(_TEXT(MESSAGE_BULLETIN_NOT_AUTH));

    auto&& resp = co_await this->post<internal_reqs::DeleteArticle, internal_resp::DeleteArticle>(
        "internal",
        "/bulletin/delete",
        internal_reqs::DeleteArticle{id, ch.id()});

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