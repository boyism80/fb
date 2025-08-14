#include <fb/game/handler/protocol/bulletin.h>
#include <fb/game/server.h>

fb::game::handler::protocol::bulletin::bulletin(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::bulletin>(server)
{ }

async::task<bool> fb::game::handler::protocol::bulletin::handle(fb::socket<character>&                 session,
                                                                fb::protocol::game::request::bulletin& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this();
    switch (request.action)
    {
    case BULLETIN_ACTION::SECTIONS:
    {
        ch->show_bulletin();
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

                ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
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

                ch->show_bulletin(model, articles, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
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
                auto flag = MAIL_BUTTON_ENABLE::NEW;
                ch->show_mail_box(resp.mail, flag);
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

                ch->show_bulletin(article, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
        }
    }
    break;

    case BULLETIN_ACTION::WRITE:
    {
        try
        {
            co_await this->server.write_bulletin(*ch, request.section, request.title, request.contents);
            co_await this->server.threads.switching(weak);

            ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_WRITE), true, false);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, false);
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
                ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, true);
            }
            else
            {
                co_await this->server.delete_bulletin(*ch, request.section, request.article);
                co_await this->server.threads.switching(weak);

                ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, false);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
        }
    }
    break;

    case BULLETIN_ACTION::MAIL:
    {
        try
        {
            auto&& resp = co_await this->server.mail_list(*ch, 0xFFFF, 20); // TODO: 20 -> const
            co_await this->server.threads.switching(weak);

            this->server.assert_mail(resp.error);
            ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, true);
        }
    }
    break;

    case BULLETIN_ACTION::SEND_MAIL:
    {
        try
        {
            auto&& resp = co_await this->server.send_mail(*ch, request.user, request.title, request.contents);
            co_await this->server.threads.switching(weak);
            ch->show_bulletin_message("우편을 보냈습니다.", true, true);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, true);
        }
    }
    break;

    default:
        co_return false;
    }

    co_return true;
}
