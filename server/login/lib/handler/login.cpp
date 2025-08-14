#include <fb/login/handler/login.h>

using namespace fb::login::handler;

login::login(fb::login::server& server) :
    fb::handler<fb::login::server, fb::protocol::login::request::login>(server)
{ }

async::task<bool> login::handle(fb::socket<fb::login::session>& session, fb::protocol::login::request::login& request)
{
    auto weak  = session.weak_from_this_as<fb::socket<fb::login::session>>();
    auto delay = fb::config<uint32_t>("transfer delay");
    auto name  = std::string(request.id);
    auto pw    = std::string(request.pw);
    co_await this->server.sleep(std::chrono::seconds(delay));

    auto fd = session.fd();
    try
    {
        this->server.assert_account(name, pw);

        auto&& response =
            co_await this->server.http.get<internal::response::GetUid>("internal", std::format("/user/uid/{}", name));
        co_await this->server.threads.switching(weak);

        if (response.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        auto   uid       = response.uid;
        auto&& response2 = co_await this->server.http.post("internal", "/user/authenticate", Authenticate{uid, pw});
        co_await this->server.threads.switching(weak);

        switch (response2.error_code)
        {
        case 1:
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        case 2:
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_PASSWORD));
        }

        auto   map       = response2.map;
        auto&& response3 = co_await this->server.http.post(
            "internal",
            "/in-game/transfer",
            Transfer{fb::protocol::internal::Service ::Game, this->server.model.map[map].host, name, true});
        co_await this->server.threads.switching(weak);

        switch (static_cast<ERROR_CODE>(response3.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw id_exception(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

        case ERROR_CODE::ALREADY_LOGIN:
            throw id_exception("이미 접속중입니다.");

        default:
            throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), response3.error));
        }

        session.send(response::message("", 0x00));
        auto parameter = fb::stream();
        auto writer    = fb::stream_writer<big_endian>(parameter);
        writer.write<uint32_t>(uid);
        writer.write<std::string>(name);
        writer.write<uint8_t>(0);
        std::ignore = this->server.transfer(session, response3.ip, response3.port, internal::Service::Login, parameter);
        co_return true;
    }
    catch (login_exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), e.type()));
    }
    catch (boost::system::error_code& e)
    {
        if (weak.expired() == false)
            session.send(response::message(std::format("({})", e.value()), 0x0E));
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            session.send(response::message(e.what(), 0x0E));
    }

    co_return true;
}
