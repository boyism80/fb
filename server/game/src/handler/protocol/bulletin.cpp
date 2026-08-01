#include <fb/game/handler/protocol/bulletin.h>
#include <fb/game/server.h>
#include <fb/game/bulletin.h>
#include <fb/game/mail_box.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game::handler::protocol;
using table = fb::model::table;
using namespace fb::model::enum_value;

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
    auto mail  = request.section == 0xFFFF;
    auto error = std::optional<std::string>{};
    try
    {
        if (mail)
        {
            auto&& summaries = co_await this->server.mail.list(*ch, request.offset, 20);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            auto flag = MAIL_BUTTON_ENABLE::NONE;
            if (ptr->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
                flag |= MAIL_BUTTON_ENABLE::NEW;

            ptr->mail_box.show(summaries, flag);
        }
        else
        {
            auto   section  = request.section;
            auto&& articles = co_await this->server.bulletin.list(request.section, request.offset);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            auto  bulletin_table = table::bulletin;
            auto& model          = bulletin_table[section];
            auto  flag           = BULLETIN_BUTTON_ENABLE::UP;
            if (ptr->condition(model.condition))
                flag |= BULLETIN_BUTTON_ENABLE::WRITE;

            ptr->bulletin.show(model, articles, flag);
        }
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->message(error.value());
    }
}

async::task<void> bulletin::handle_article(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto mail  = request.section == 0xFFFF;
    auto error = std::optional<std::string>{};
    try
    {
        if (mail)
        {
            auto&& mail = co_await this->server.mail.read(*ch, request.article);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;
            auto flag = MAIL_BUTTON_ENABLE::UP;
            if (ptr->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
                flag |= MAIL_BUTTON_ENABLE::NEW;

            ptr->mail_box.show(mail, flag);

            if (!mail.read)
            {
                auto log_data              = Json::Value();
                log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
                log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
                log_data["mail_id"]        = request.article;
                log_data["sender_name"]    = UTF8(mail.sender, PLATFORM::WINDOWS);
                this->server.log.write("mail_read", log_data);
            }
        }
        else
        {
            auto&& article = co_await this->server.bulletin.read(request.section, request.article);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            auto flag = BULLETIN_BUTTON_ENABLE::NONE;
            if (article.next)
                flag |= BULLETIN_BUTTON_ENABLE::NEXT;

            if (ptr->condition(table::bulletin[article.section].condition))
                flag |= BULLETIN_BUTTON_ENABLE::WRITE;

            ptr->bulletin.show(article, flag);
        }
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            if (mail)
                ptr->mail_box.message(error.value(), false, BULLETIN_MESSAGE_TYPE::READ);
            else
                ptr->bulletin.message(error.value(), false, BULLETIN_MESSAGE_TYPE::READ);
        }
    }
}

async::task<void> bulletin::handle_write(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto error = std::optional<std::string>{};
    try
    {
        co_await this->server.bulletin.write(*ch, request.section, request.title, request.contents);
        co_await this->server.threads.switching(weak);
        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;

        ptr->bulletin.message(_TEXT(MESSAGE_BULLETIN_WRITE), true, BULLETIN_MESSAGE_TYPE::WRITE);

        auto log_data              = Json::Value();
        log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
        log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
        log_data["section"]        = request.section;
        log_data["title"]          = UTF8(request.title, PLATFORM::WINDOWS);
        this->server.log.write("bulletin_write", log_data);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->bulletin.message(error.value(), false, BULLETIN_MESSAGE_TYPE::WRITE);
    }
}

async::task<void> bulletin::handle_delete(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto mail  = request.section == 0xFFFF;
    auto error = std::optional<std::string>{};
    try
    {
        if (mail)
        {
            co_await this->server.mail.remove(*ch, request.article);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;
            ptr->mail_box.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, BULLETIN_MESSAGE_TYPE::DELETE);

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
            log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
            log_data["mail_id"]        = request.article;
            this->server.log.write("mail_delete", log_data);
        }
        else
        {
            co_await this->server.bulletin.remove(*ch, request.section, request.article);
            co_await this->server.threads.switching(weak);
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;
            ptr->bulletin.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, BULLETIN_MESSAGE_TYPE::DELETE);

            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ptr->id);
            log_data["character_name"] = UTF8(ptr->name(), PLATFORM::WINDOWS);
            log_data["section"]        = request.section;
            log_data["article_id"]     = request.article;
            this->server.log.write("bulletin_delete", log_data);
        }
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
        {
            if (mail)
                ptr->mail_box.message(error.value(), false, BULLETIN_MESSAGE_TYPE::DELETE);
            else
                ptr->bulletin.message(error.value(), false, BULLETIN_MESSAGE_TYPE::DELETE);
        }
    }
}

async::task<void> bulletin::handle_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto error = std::optional<std::string>{};
    try
    {
        auto&& summaries = co_await this->server.mail.list(*ch, 0xFFFF, fb::model::const_value::mail::COUNT_PER_PAGE);
        co_await this->server.threads.switching(weak);
        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;

        auto flag = MAIL_BUTTON_ENABLE::NONE;
        if (ptr->level() >= fb::model::const_value::mail::REQUIRED_LEVEL)
            flag |= MAIL_BUTTON_ENABLE::NEW;

        ptr->mail_box.show(summaries, flag);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->message(error.value());
    }
}

async::task<void> bulletin::handle_send_mail(character* ch, std::weak_ptr<character> weak, game_reqs::bulletin& request)
{
    auto error = std::optional<std::string>{};
    try
    {
        this->server.mail.send(*ch, request.user, request.title, request.contents);
        co_await this->server.threads.switching(weak);
        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;
        ptr->mail_box.message(_TEXT(MESSAGE_MAIL_SENT), true, BULLETIN_MESSAGE_TYPE::WRITE);

        auto log_data             = Json::Value();
        log_data["sender_id"]     = static_cast<Json::Int64>(ptr->id);
        log_data["sender_name"]   = UTF8(ptr->name(), PLATFORM::WINDOWS);
        log_data["receiver_name"] = UTF8(request.user, PLATFORM::WINDOWS);
        log_data["title"]         = UTF8(request.title, PLATFORM::WINDOWS);
        this->server.log.write("mail_write", log_data);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.has_value())
    {
        auto ptr = weak.lock();
        if (ptr != nullptr)
            ptr->mail_box.message(error.value(), false, BULLETIN_MESSAGE_TYPE::WRITE);
    }
}
