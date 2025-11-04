#include <fb/game/handler/protocol/bulletin.h>
#include <fb/game/server.h>

fb::game::handler::protocol::bulletin::bulletin(fb::game::server& server) :
    fb::handler::protocol<fb::game::server, fb::protocol::game::request::bulletin>(server)
{ }

async::task<bool> fb::game::handler::protocol::bulletin::handle(fb::socket<character>& session, fb::protocol::game::request::bulletin& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this();
    switch (request.action)
    {
    case BULLETIN_ACTION::SECTIONS:
    {
        ch->bulletin.show();
    }
    break;

    case BULLETIN_ACTION::ARTICLES:
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
                    dao.push_back(mail_box::summary{summary.id, summary.user, summary.sender, summary.sender_name, summary.read, summary.title, summary.created_date});
                }

                ch->mail_box.show(dao, flag);
            }
            else
            {
                auto   section  = request.section;
                auto&& articles = co_await this->server.bulletin_list(request.section, request.offset);
                co_await this->server.threads.switching(weak);

                auto& model = this->server.model.bulletin[section];
                auto  flag  = BULLETIN_BUTTON_ENABLE::UP;
                if (ch->condition(model.condition))
                    flag |= BULLETIN_BUTTON_ENABLE::WRITE;

                ch->bulletin.show(model, articles, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
            {
                if (mail)
                    ch->mail_box.message(e.what(), false);
                else
                    ch->bulletin.message(e.what(), false);
            }
        }
    }
    break;

    case BULLETIN_ACTION::ARTICLE:
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
                                                 resp.mail.sender_name,
                                                 resp.mail.title,
                                                 resp.mail.contents,
                                                 resp.mail.read,
                                                 resp.mail.created_date},
                                  flag);
            }
            else
            {
                auto&& article = co_await this->server.read_bulletin(request.section, request.article);
                co_await this->server.threads.switching(weak);

                auto flag = BULLETIN_BUTTON_ENABLE::NONE;
                if (article.next)
                    flag |= BULLETIN_BUTTON_ENABLE::NEXT;

                if (ch->condition(this->server.model.bulletin[article.section].condition) == false)
                    flag |= BULLETIN_BUTTON_ENABLE::WRITE;

                ch->bulletin.show(article, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
            {
                if (mail)
                    ch->mail_box.message(e.what(), false);
                else
                    ch->bulletin.message(e.what(), false);
            }
        }
    }
    break;

    case BULLETIN_ACTION::WRITE:
    {
        try
        {
            co_await this->server.write_bulletin(*ch, request.section, request.title, request.contents);
            co_await this->server.threads.switching(weak);

            ch->bulletin.message(_TEXT(MESSAGE_BULLETIN_WRITE), true);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->bulletin.message(e.what(), false);
        }
    }
    break;

    case BULLETIN_ACTION::DELETE:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->server.delete_mail(*ch, request.article);
                co_await this->server.threads.switching(weak);
                ch->mail_box.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true);
            }
            else
            {
                co_await this->server.delete_bulletin(*ch, request.section, request.article);
                co_await this->server.threads.switching(weak);
                ch->bulletin.message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
            {
                if (mail)
                    ch->mail_box.message(e.what(), false);
                else
                    ch->bulletin.message(e.what(), false);
            }
        }
    }
    break;

    case BULLETIN_ACTION::MAIL:
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
                dao.push_back(mail_box::summary{summary.id, summary.user, summary.sender, summary.sender_name, summary.read, summary.title, summary.created_date});
            }

            ch->mail_box.show(dao, flag);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->mail_box.message(e.what(), false);
        }
    }
    break;

    case BULLETIN_ACTION::SEND_MAIL:
    {
        try
        {
            auto&& resp = co_await this->server.send_mail(*ch, request.user, request.title, request.contents);
            co_await this->server.threads.switching(weak);
            ch->mail_box.message("우편을 보냈습니다.", true, false);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->mail_box.message(e.what(), false);
        }
    }
    break;

    default:
        co_return false;
    }

    co_return true;
}
