#include <fb/game/handler/protocol/bulletin.h>
#include <fb/game/server.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game::handler::protocol;
using table = fb::model::table;

namespace game_reqs = fb::protocol::game::request;
namespace game_resp = fb::protocol::game::response;

bulletin::bulletin(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, game_reqs::bulletin>(server)
{ }

async::task<bool> bulletin::handle(fb::socket<character>& session, game_reqs::bulletin& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    switch (request.action)
    {
    case BULLETIN_ACTION::SECTIONS:
        co_await handle_sections(ch);
        break;

    case BULLETIN_ACTION::ARTICLES:
        co_await handle_articles(ch, weak, request);
        break;

    case BULLETIN_ACTION::ARTICLE:
        co_await handle_article(ch, weak, request);
        break;

    case BULLETIN_ACTION::WRITE:
        co_await handle_write(ch, weak, request);
        break;

    case BULLETIN_ACTION::DELETE:
        co_await handle_delete(ch, weak, request);
        break;

    case BULLETIN_ACTION::MAIL:
        co_await handle_mail(ch, weak, request);
        break;

    case BULLETIN_ACTION::SEND_MAIL:
        co_await handle_send_mail(ch, weak, request);
        break;

    default:
        co_return false;
    }

    co_return true;
}

async::task<void> bulletin::handle_sections(character* ch)
{
    ch->bulletin.show();
    co_return;
}

async::task<void> bulletin::handle_articles(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto mail = request.section == 0xFFFF;
    try
    {
        if (mail)
        {
            auto&& resp = co_await this->server.mail_list(*ch, request.offset, 20);
            co_await this->server.threads.switching(weak);

            auto flag = MAIL_BUTTON_ENABLE::NONE;
            if (ch->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
                flag |= MAIL_BUTTON_ENABLE::NEW;

            auto dao = std::vector<mail_box::summary>();
            for (auto& summary : resp.summary_list)
            {
                dao.push_back(mail_box::summary{summary.id,
                                                summary.user,
                                                summary.sender,
                                                summary.read,
                                                summary.title,
                                                summary.created_date});
            }

            ch->mail_box.show(dao, flag);
        }
        else
        {
            auto   section  = request.section;
            auto&& articles = co_await this->server.bulletin_list(request.section, request.offset);
            co_await this->server.threads.switching(weak);

            auto& model = table::bulletin[section];
            auto  flag  = BULLETIN_BUTTON_ENABLE::UP;
            if (ch->condition(model.condition))
                flag |= BULLETIN_BUTTON_ENABLE::WRITE;

            ch->bulletin.show(model, articles, flag);
        }
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            ch->message(e.what());
    }
}

async::task<void> bulletin::handle_article(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto mail = request.section == 0xFFFF;
    try
    {
        if (mail)
        {
            auto&& resp = co_await this->server.read_mail(*ch, request.article);
            co_await this->server.threads.switching(weak);
            auto flag = MAIL_BUTTON_ENABLE::UP;
            if (ch->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
                flag |= MAIL_BUTTON_ENABLE::NEW;

            ch->mail_box.show(mail_box::mail{resp.mail.id,
                                             resp.mail.user,
                                             resp.mail.sender,
                                             resp.mail.title,
                                             resp.mail.contents,
                                             resp.mail.read,
                                             resp.mail.created_date},
                              flag);

            if (!resp.mail.read)
            {
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
                log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
                log_data["mail_id"]        = request.article;
                log_data["sender_name"]    = UTF8(resp.mail.sender, PLATFORM::WINDOWS);
                this->server.log.write("mail_read", log_data);
            }
        }
        else
        {
            auto&& article = co_await this->server.read_bulletin(request.section, request.article);
            co_await this->server.threads.switching(weak);

            auto flag = BULLETIN_BUTTON_ENABLE::NONE;
            if (article.next)
                flag |= BULLETIN_BUTTON_ENABLE::NEXT;

            if (ch->condition(table::bulletin[article.section].condition))
                flag |= BULLETIN_BUTTON_ENABLE::WRITE;

            ch->bulletin.show(article, flag);
        }
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
        {
            if (mail)
                ch->mail_box.message(e.what(), false, true);
            else
                ch->bulletin.message(e.what(), false, true);
        }
    }
}

async::task<void> bulletin::handle_write(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    try
    {
        co_await this->server.write_bulletin(*ch, request.section, request.title, request.contents);
        co_await this->server.threads.switching(weak);

        ch->bulletin.message(_TEXT(MESSAGE_BULLETIN_WRITE), true, false);

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
        log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
        log_data["section"]        = request.section;
        log_data["title"]          = UTF8(request.title, PLATFORM::WINDOWS);
        this->server.log.write("bulletin_write", log_data);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            ch->bulletin.message(e.what(), false, false);
    }
}

async::task<void> bulletin::handle_delete(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto mail = request.section == 0xFFFF;
    try
    {
        if (mail)
        {
            auto&& resp = co_await this->server.delete_mail(*ch, request.article);
            co_await this->server.threads.switching(weak);
            ch->mail_box.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, false);

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
            log_data["mail_id"]        = request.article;
            this->server.log.write("mail_delete", log_data);
        }
        else
        {
            co_await this->server.delete_bulletin(*ch, request.section, request.article);
            co_await this->server.threads.switching(weak);
            ch->bulletin.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, true);

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch->id);
            log_data["character_name"] = UTF8(ch->name(), PLATFORM::WINDOWS);
            log_data["section"]        = request.section;
            log_data["article_id"]     = request.article;
            this->server.log.write("bulletin_delete", log_data);
        }
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
        {
            if (mail)
                ch->mail_box.message(e.what(), false, true);
            else
                ch->bulletin.message(e.what(), false, true);
        }
    }
}

async::task<void> bulletin::handle_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    try
    {
        auto&& resp = co_await this->server.mail_list(*ch, 0xFFFF, fb::model::const_value::mail::COUNT_PER_PAGE);
        co_await this->server.threads.switching(weak);

        this->server.assert_mail(resp.error);
        auto flag = MAIL_BUTTON_ENABLE::NONE;
        if (ch->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
            flag |= MAIL_BUTTON_ENABLE::NEW;

        auto dao = std::vector<mail_box::summary>();
        for (auto& summary : resp.summary_list)
        {
            dao.push_back(mail_box::summary{summary.id,
                                            summary.user,
                                            summary.sender,
                                            summary.read,
                                            summary.title,
                                            summary.created_date});
        }

        ch->mail_box.show(dao, flag);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            ch->message(e.what());
    }
}

async::task<void> bulletin::handle_send_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    try
    {
        auto&& resp = co_await this->server.send_mail(*ch, request.user, request.title, request.contents);
        co_await this->server.threads.switching(weak);
        ch->mail_box.message(_TEXT(MESSAGE_MAIL_SENT), true, false);

        auto log_data             = Json::Value();
        log_data["sender_id"]     = static_cast<Json::Int64>(ch->id);
        log_data["sender_name"]   = UTF8(ch->name(), PLATFORM::WINDOWS);
        log_data["receiver_name"] = UTF8(request.user, PLATFORM::WINDOWS);
        log_data["title"]         = UTF8(request.title, PLATFORM::WINDOWS);
        this->server.log.write("mail_write", log_data);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            ch->mail_box.message(e.what(), false, true);
    }
}
