#include <boost/asio/high_resolution_timer.hpp>
#include <fb/login/server.h>
#include <fb/login/exception.h>
#include <fb/login/handler.h>
#include <fb/log_collector.h>
#include <fb/console.h>
#include <fb/encoding.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/model/loader.h>
#include <format>

using namespace fb::login;

namespace internal_reqs = fb::protocol::internal::request;

fb::login::server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<session>(io_context, "LOGIN", port, fb::config<uint32_t>("http:max_concurrent", 500)),
    log(fb::config<std::string>("amqp:log:ip"),
        fb::config<uint16_t>("amqp:log:port"),
        fb::config<std::string>("amqp:log:uid"),
        fb::config<std::string>("amqp:log:pwd"),
        std::to_string(fb::config<uint32_t>("id")),
        fb::config<std::string>("name"),
        fb::config<uint32_t>("world"))
{
    this->handler.protocol.bind<fb::login::handler::protocol::login>();
    this->handler.protocol.bind<fb::login::handler::protocol::agreement>();
    this->handler.protocol.bind<fb::login::handler::protocol::create_account>();
    this->handler.protocol.bind<fb::login::handler::protocol::complete>();
    this->handler.protocol.bind<fb::login::handler::protocol::change_password>();
}

fb::login::server::~server()
{ }

bool fb::login::server::decrypt_policy(uint8_t opcode) const
{
    switch (opcode)
    {
    case fb::protocol::login::request::agreement::opcode:
        return false;

    default:
        return true;
    }
}

async::task<void> fb::login::server::on_start()
{
#ifdef _WIN32
    fb::model::option::decoding(fb::cp949);
    fb::model::option::encoding(fb::utf8);
    flatbuffers::option::encoding(fb::utf8);
    flatbuffers::option::decoding(fb::cp949);
#endif

    co_await fb::model::loader(*this).run();

    co_await fb::acceptor<session>::on_start();

    fb::console::set_mode(fb::console::mode::plain);
#ifdef _WIN32
    fb::model::option::decoding(nullptr);
#endif

    this->bind_timer<fb::login::handler::timer::heart_beat>(1s);
    this->handler.amqp.bind<fb::login::handler::amqp::shutdown>("fb.global"); // Shutdown: all servers
    this->handler.amqp.bind<fb::login::handler::amqp::set_datetime>(
        std::format("fb.{}.global", fb::config<uint32_t>("world")));
}

async::task<void> fb::login::server::update_status()
{
    try
    {
        auto world  = fb::config<uint32_t>("world");
        std::ignore = co_await this->http.post("internal",
                                               "/server/heartbeat",
                                               internal_reqs::Heartbeat{world,
                                                                        internal::Service::Login,
                                                                        this->id(),
                                                                        this->name(),
                                                                        fb::config<std::string_view>("ip"),
                                                                        fb::config<uint16_t>("port")});
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to send heartbeat: {}", e.what());
    }
}

const fb::protocol::login::response::terms_agreement& fb::login::server::agreement() const
{
    return this->_agreement;
}

void fb::login::server::assert_account(std::string_view id, std::string_view pw) const
{
    auto id_str    = std::string(id);
    auto cp949     = CP949(id_str);
    auto name_size = cp949.length();

    if (name_size < fb::config<int>("name_size:min") || name_size > fb::config<int>("name_size:max"))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name must be full-hangul characters
    if (fb::config<bool>("allow_foreign_name") == false && assert_korean(cp949) == false)
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    if (fb::model::table::blocked_name.contains_substring(id_str))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    if (fb::model::table::blocked_word.contains_substring(id_str))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Read character's password
    if (pw.length() < fb::config<int>("pw_size:min") || pw.length() > fb::config<int>("pw_size:max"))
        throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));
}

async::task<void> fb::login::server::on_accepted(fb::socket<session>& socket)
{
    auto data = std::make_shared<session>();
    socket.data(data);
    co_return;
}

async::task<bool> fb::login::server::on_connected(fb::socket<session>& socket)
{
    co_return true;
}

async::task<bool> fb::login::server::on_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

void fb::login::server::on_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.global"); // Shutdown: all servers
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.{}.global", fb::config<uint32_t>("world")));
}