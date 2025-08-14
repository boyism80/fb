#include <fb/game/handler/update_option.h>
#include <fb/game/server.h>

using namespace fb::game::handler;

update_option::update_option(fb::game::server& server) :
    fb::handler<fb::game::server, fb::protocol::game::request::update_option>(server)
{ }

async::task<bool> update_option::handle(fb::socket<character>&                      session,
                                        fb::protocol::game::request::update_option& request)
{
    auto ch = session.data();
    if (ch->inited() == false)
        co_return true;

    auto weak   = ch->weak_from_this();
    auto option = OPTION(request.option);
    switch (option)
    {
    case OPTION::EXTENSION:
        if (request.ride)
        {
            if (ch->state() == STATE::RIDING)
                ch->unride();
            else
                ch->ride();
        }
        else
        {
            ch->update_option();
        }
        break;

    default:
        auto   next = !ch->option(option);
        auto&& resp = co_await this->server.http.post("internal",
                                                      "/user/option",
                                                      SetOption{ch->id(), static_cast<uint8_t>(option), next});
        co_await this->server.threads.switching(weak);

        if (resp.success == false)
            ch->message("설정을 변경하지 못했습니다.");

        if (option == OPTION::GROUP && !next)
            co_await this->server.leave_group(*ch);

        ch->option(option, next);
        break;
    }
    co_return true;
}
