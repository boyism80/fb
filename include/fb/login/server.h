#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <json/json.h>
#include <regex>
#include <ctime>
#include <zlib.h>
#include <fb/socket.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/acceptor.h>
#include <fb/model/model.h>
#include <fb/login/session.h>
#include <fb/login/gateway.h>
#include <fb/login/protocol.h>
#include <fb/redis.h>

#define MAX_NXCLUB_SIZE 14

using namespace fb::protocol::login;
using namespace fb::model::enum_value;
using namespace fb::protocol::internal::request;

namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

REGISTER_RESPONSE(fb::protocol::internal::request::ReserveName, fb::protocol::internal::response::ReserveName)
REGISTER_RESPONSE(fb::protocol::internal::request::InitCharacter, fb::protocol::internal::response::InitCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::MakeCharacter, fb::protocol::internal::response::MakeCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::Authenticate, fb::protocol::internal::response::Authenticate)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangePw, fb::protocol::internal::response::ChangePw)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)

namespace fb::login {

class login_exception : public std::runtime_error
{
private:
    uint8_t _exc_type;

public:
    login_exception(uint8_t type, const std::string& what) :
        std::runtime_error(what),
        _exc_type(type)
    { }

public:
    uint8_t type() const
    {
        return this->_exc_type;
    }
};

class id_exception : public login_exception
{
public:
    id_exception(const std::string& what) :
        login_exception(0x0E, what)
    { }
};

class pw_exception : public login_exception
{
public:
    pw_exception(const std::string& what) :
        login_exception(0x0F, what)
    { }
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(const std::string& what) :
        login_exception(0x05, what)
    { }
};

class btd_exception : public login_exception
{
public:
    btd_exception() :
        login_exception(0x1F, _TEXT(MESSAGE_ACCOUNT_INVALID_BIRTHDAY))
    { }
};

class server : public fb::acceptor<fb::login::session>
{
private:
    fb::protocol::login::response::agreement _agreement = CP949(fb::config<std::string>("agreement"), PLATFORM::BOTH);
    std::vector<std::string>                 _forbiddens;
    std::vector<boost::asio::deadline_timer> _timers;
    fb::redis                                _redis;

    bool is_forbidden_impl(const std::string& str) const;

public:
    fb::model::model model;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    ~server();

    const fb::protocol::login::response::agreement& agreement() const;
    bool                                            is_forbidden(const std::string& str) const;
    void                                            assert_account(const std::string& id, const std::string& pw) const;

protected:
    bool                                decrypt_policy(uint8_t cmd) const override final;
    void                                handle_init_amqp(fb::amqp::socket& amqp) override final;
    [[nodiscard]] async::task<void>     handle_start() final;
    std::shared_ptr<fb::login::session> handle_accepted(fb::socket<fb::login::session>& socket) override final;
    [[nodiscard]] async::task<bool>     handle_connected(fb::socket<fb::login::session>& socket) final;
    [[nodiscard]] async::task<bool>     handle_disconnected(fb::socket<fb::login::session>& socket) final;

protected:
    fb::protocol::internal::Service service() const override final
    {
        return fb::protocol::internal::Service::Login;
    };

public:
    void update_status();
};

} // namespace fb::login

#endif // __FB_LOGIN_H__
