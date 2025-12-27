#include <fb/login/handler/protocol/complete.h>
#include <fb/model/model.h>

using namespace fb::login::handler::protocol;

namespace internal_reqs = fb::protocol::internal::request;

complete::complete(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, fb::protocol::login::request::complete>(server)
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

        auto&& response = co_await this->server.http.post("internal",
                                                          "/account/make",
                                                          internal_reqs::MakeCharacter{session_data->pk,
                                                                                       request.hair,
                                                                                       request.sex,
                                                                                       request.nation,
                                                                                       request.creature});
        co_await this->server.threads.switching(weak);

        if (response.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_ALREADY_EXISTS));

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
