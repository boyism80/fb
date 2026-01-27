#include <fb/login/handler/protocol/login.h>
#include <fb/encoding.h>
#include <json/json.h>
#include <format>

using namespace fb::login::handler::protocol;
using table = fb::model::table;

namespace internal_reqs = fb::protocol::internal::request;

login::login(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, fb::protocol::login::request::login>(server)
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

        auto   world = fb::config<uint32_t>("world");
        auto&& resp1 =
            co_await this->server.http.get<internal::response::GetUid>("internal",
                                                                       std::format("/account/{}/uid/{}", world, name));
        co_await this->server.threads.switching(weak);

        if (resp1.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        auto   uid   = resp1.uid;
        auto&& resp2 = co_await this->server.http.post("internal",
                                                       "/account/authenticate",
                                                       internal_reqs::Authenticate{world, uid, pw});
        co_await this->server.threads.switching(weak);

        switch (resp2.error_code)
        {
        case 1:
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        case 2:
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_PASSWORD));
        }

        auto   map   = resp2.map;
        auto&& resp3 = co_await this->server.http.post(
            "internal",
            "/in-game/transfer",
            internal_reqs::Transfer{world, fb::protocol::internal::Service ::Game, table::map[map].host, name, true});
        co_await this->server.threads.switching(weak);

        switch (static_cast<ERROR_CODE>(resp3.error))
        {
        case ERROR_CODE::NONE:
            break;

        case ERROR_CODE::SERVER_NOT_READY:
            throw id_exception(_TEXT(MESSAGE_NOT_READY_GAME_SERVER));

        case ERROR_CODE::ALREADY_LOGIN:
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_ALREADY_LOGIN));

        case ERROR_CODE::BANNED:
            throw id_exception(build_ban_message(resp3.ban_reason, resp3.ban_expire_date));

        default:
            throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), resp3.error));
        }

        // Log game server entry event
        auto log_data              = Json::Value();
        log_data["account_name"]   = UTF8(name, PLATFORM::WINDOWS);
        log_data["uid"]            = static_cast<Json::Int64>(uid);
        log_data["game_server_id"] = static_cast<Json::Int64>(table::map[map].host);
        log_data["map"]            = static_cast<Json::Int64>(map);
        log_data["ip"]             = resp3.ip;
        log_data["port"]           = static_cast<Json::Int64>(resp3.port);
        this->server.log.write("game_server_entry", log_data);

        session.send(response::message("", 0x00));
        auto parameter = fb::stream();
        auto writer    = fb::stream_writer<big_endian>(parameter);
        writer.write<uint32_t>(uid);
        writer.write<std::string>(name);
        writer.write<uint8_t>(0);
        std::ignore = this->server.transfer(session, resp3.ip, resp3.port, internal::Service::Login, parameter);
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

std::string login::build_ban_message(const std::string& reason, const std::optional<std::string>& expire_date)
{
    auto ban_message = std::string(_TEXT(MESSAGE_ACCOUNT_BANNED));
    if (!reason.empty())
    {
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_REASON), reason);
    }
    if (expire_date.has_value())
    {
        ban_message += std::format(_TEXT(MESSAGE_ACCOUNT_BAN_EXPIRE_DATE), expire_date.value());
    }
    else
    {
        ban_message += _TEXT(MESSAGE_ACCOUNT_BAN_PERMANENT);
    }
    return ban_message;
}
