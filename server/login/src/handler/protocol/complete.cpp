#include <fb/login/handler/protocol/complete.h>
#include <fb/login/exception.h>
#include <fb/model/model.h>

using namespace fb::login::handler::protocol;
using namespace fb::model;

namespace internal_reqs = fb::protocol::internal::request;

template <fb::protocol::CLIENT_VERSION V>
complete<V>::complete(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::complete<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> complete<V>::handle(fb::socket<fb::login::session>& session, login_reqs::complete<V>& request)
{
    auto fd   = session.fd();
    auto weak = session.weak_from_this_as<fb::socket<fb::login::session>>();

    try
    {
        auto session_data = session.data();
        if (session_data == nullptr)
            throw std::runtime_error("session is not established");

        if (session_data->pk == -1)
            throw std::exception();

        auto nation = static_cast<NATION>(request.nation);
        if (nation != NATION::GOGURYEO && nation != NATION::BUYEO)
            throw id_exception(_TEXT(MESSAGE_CLIENT_NATION_INVALID));

        auto gender = static_cast<GENDER>(request.gender);
        if (gender != GENDER::MALE && gender != GENDER::FEMALE)
            throw id_exception(_TEXT(MESSAGE_CLIENT_GENDER_INVALID));

        auto divine_beast = static_cast<DIVINE_BEAST>(request.divine_beast);
        if (divine_beast != DIVINE_BEAST::VERMILION_BIRD && divine_beast != DIVINE_BEAST::WHITE_TIGER &&
            divine_beast != DIVINE_BEAST::BLACK_TORTOISE && divine_beast != DIVINE_BEAST::AZURE_DRAGON)
            throw id_exception(_TEXT(MESSAGE_CLIENT_DIVINE_BEAST_INVALID));

        auto   world    = fb::config<uint32_t>("world");
        auto&& response = co_await this->server.http.post("internal",
                                                          "/account/make",
                                                          internal_reqs::MakeCharacter{world,
                                                                                       session_data->pk,
                                                                                       request.hair,
                                                                                       request.gender,
                                                                                       request.nation,
                                                                                       request.divine_beast});
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

template class complete<fb::protocol::CLIENT_VERSION::v550>;
template class complete<fb::protocol::CLIENT_VERSION::v565>;
template class complete<fb::protocol::CLIENT_VERSION::v651>;
