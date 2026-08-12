#include <fb/login/handler/protocol/change_password.h>
#include <fb/login/exception.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::login::handler::protocol;

namespace internal_reqs = fb::protocol::internal::request;

template <fb::protocol::CLIENT_VERSION V>
change_password<V>::change_password(fb::login::server& server) :
    fb::handler::protocol<fb::login::server, login_reqs::update_pw<V>>(server)
{ }

template <fb::protocol::CLIENT_VERSION V>
async::task<bool> change_password<V>::handle(fb::socket<fb::login::session>& session, login_reqs::update_pw<V>& request)
{
    auto weak = session.weak_from_this_as<fb::socket<fb::login::session>>();
    try
    {
        // co_await this->_auth_service.change_pw(request.name, request.pw, request.new_pw, request.birthday);

        auto delay = fb::config<uint32_t>("transfer delay");
        co_await this->server.sleep(std::chrono::seconds(delay));

        auto name_cp949 = CP949(request.name);
        if (name_cp949.length() < fb::config("name_size:min").asInt() ||
            name_cp949.length() > fb::config("name_size:max").asInt())
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        // Name must be Hangul syllables (no jamo)
        if (fb::config<bool>("login:account option:allow_foreign_name") == false && assert_korean(name_cp949) == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        // Name cannot contains subcharacters in forbidden list
        if (fb::model::table::blocked_name->contains_substring(request.name))
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        if (fb::model::table::blocked_word->contains_substring(request.name))
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

        if (request.pw.length() < fb::config("pw_size:min").asInt() ||
            request.pw.length() > fb::config("pw_size:max").asInt())
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));

        if (request.new_pw.length() < fb::config("pw_size:min").asInt() ||
            request.new_pw.length() > fb::config("pw_size:max").asInt())
            throw newpw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));

        // TODO: Check if password is too weak
        if (request.pw == request.new_pw)
            throw newpw_exception(_TEXT(MESSAGE_ACCOUNT_NEW_PW_EQUALIZATION));

        auto   world = fb::config<uint32_t>("world");
        auto&& resp1 = co_await this->server.http.get<internal::response::GetUid>(
            "internal",
            std::format("/account/{}/uid/{}", world, request.name));
        co_await this->server.threads.switching(weak);

        if (resp1.success == false)
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        auto uid = resp1.uid;

        auto&& resp2 = co_await this->server.http.post(
            "internal",
            "/account/change-pw",
            internal_reqs::ChangePw{world, uid, request.pw, request.new_pw, request.birthday});

        co_await this->server.threads.switching(weak);

        switch (static_cast<ERROR_CODE>(resp2.error_code))
        {
        case ERROR_CODE::NOT_FOUND_CHARACTER: // id wrong
            throw id_exception(_TEXT(MESSAGE_ACCOUNT_NOT_FOUND_NAME));

        case ERROR_CODE::PASSWORD_NOT_MATCHED: // pw wrong
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_PASSWORD));

        case ERROR_CODE::BIRTHDAY_NOT_MATCHED: // birthday wrong
            throw pw_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_BIRTHDAY));
        }

        // Log password change event
        auto log_data            = Json::Value();
        log_data["account_name"] = UTF8(request.name, PLATFORM::WINDOWS);
        log_data["uid"]          = static_cast<Json::Int64>(uid);
        this->server.log.write("password_change", log_data);

        session.send(response::message((_TEXT(MESSAGE_ACCOUNT_SUCCESS_CHANGE_PASSWORD)), 0x00));
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

template class change_password<fb::protocol::CLIENT_VERSION::v550>;
template class change_password<fb::protocol::CLIENT_VERSION::v565>;
template class change_password<fb::protocol::CLIENT_VERSION::v651>;
