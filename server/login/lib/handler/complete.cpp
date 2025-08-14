#include <fb/login/handler/complete.h>

using namespace fb::login::handler;

complete::complete(fb::login::server& server) :
    fb::handler<fb::login::server, fb::protocol::login::request::complete>(server)
{ }

async::task<bool> complete::handle(fb::socket<fb::login::session>&         session,
                                   fb::protocol::login::request::complete& request)
{
    auto fd   = session.fd();
    auto weak = session.weak_from_this_as<fb::socket<fb::login::session>>();

    try
    {
        auto session_data = session.data();
        if (session_data->pk == -1)
            throw std::exception();

        auto&& response = co_await this->server.http.post(
            "internal",
            "/user/mk-ch",
            MakeCharacter{session_data->pk, request.hair, request.sex, request.nation, request.creature});
        co_await this->server.threads.switching(weak);

        if (response.success == false)
            throw id_exception("이미 존재하는 이름입니다.");

        session.send(response::message(_TEXT(MESSAGE_ACCOUNT_SUCCESS_REGISTER_ACCOUNT), 0x00));
        session_data->pk = -1;
        session_data->name.clear();
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), e.type()));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}
