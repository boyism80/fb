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
#include <fb/login/protocol.h>
#include <fb/log_collector.h>
#include <fb/meta_dat_file.h>
#include <memory>
#include <string_view>

#define MAX_NXCLUB_SIZE 14

REGISTER_RESPONSE(fb::protocol::internal::request::ReserveName, fb::protocol::internal::response::ReserveName)
REGISTER_RESPONSE(fb::protocol::internal::request::Heartbeat, fb::protocol::internal::response::Heartbeat)
REGISTER_RESPONSE(fb::protocol::internal::request::InitCharacter, fb::protocol::internal::response::InitCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::MakeCharacter, fb::protocol::internal::response::MakeCharacter)
REGISTER_RESPONSE(fb::protocol::internal::request::Authenticate, fb::protocol::internal::response::Authenticate)
REGISTER_RESPONSE(fb::protocol::internal::request::ChangePw, fb::protocol::internal::response::ChangePw)
REGISTER_RESPONSE(fb::protocol::internal::request::Transfer, fb::protocol::internal::response::Transfer)
REGISTER_RESPONSE(fb::protocol::internal::request::SetDateTime, fb::protocol::internal::response::SetDateTime)

namespace fb::login {

using namespace fb::protocol::login;
using namespace fb::model::enum_value;
using namespace fb::protocol::internal::request;

namespace internal      = fb::protocol::internal;
namespace internal_resp = fb::protocol::internal::response;

class server : public fb::acceptor<fb::login::session>
{
private:
    fb::protocol::login::response::terms_agreement _agreement{
        CP949(fb::config<std::string_view>("agreement"), PLATFORM::BOTH)};
    std::vector<boost::asio::deadline_timer> _timers;

public:
    fb::log_collector log;
    fb::meta_dat_file meta;

public:
    server(boost::asio::io_context& io_context, uint16_t port);
    server(const server&) = delete;
    server(server&&)      = delete;
    ~server();

    const fb::protocol::login::response::terms_agreement& agreement() const;
    void assert_account(std::string_view id, std::string_view pw) const;

protected:
    bool                            decrypt_policy(uint8_t opcode) const override final;
    void                            on_init_amqp(fb::amqp::socket& amqp) override final;
    [[nodiscard]] async::task<void> on_start() override final;
    [[nodiscard]] async::task<void> on_accepted(fb::socket<fb::login::session>& socket) override final;
    [[nodiscard]] async::task<bool> on_connected(fb::socket<fb::login::session>& socket) override final;
    [[nodiscard]] async::task<bool> on_disconnected(fb::socket<fb::login::session>& socket) override final;

protected:
    fb::protocol::internal::Service service() const override final
    {
        return fb::protocol::internal::Service::Login;
    };

public:
    async::task<void> update_status();
};

} // namespace fb::login

#endif // __FB_LOGIN_H__
