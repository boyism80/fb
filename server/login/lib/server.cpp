#include <boost/asio/high_resolution_timer.hpp>
#include <fb/login/server.h>
#include <fb/login/handler.h>
#include <fb/log_collector.h>
#include <format>

using namespace fb::login;

namespace internal_reqs = fb::protocol::internal::request;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<session>(io_context, "LOGIN", port),
    log(fb::config<std::string>("amqp:log:ip"),
        fb::config<uint16_t>("amqp:log:port"),
        fb::config<std::string>("amqp:log:uid"),
        fb::config<std::string>("amqp:log:pwd"),
        std::to_string(fb::config<uint32_t>("id")),
        fb::config<std::string>("name"),
        fb::config<size_t>("amqp:log:queue_size"))
{
    this->handler.protocol.bind<fb::login::handler::protocol::login>();
    this->handler.protocol.bind<fb::login::handler::protocol::agreement>();
    this->handler.protocol.bind<fb::login::handler::protocol::create_account>();
    this->handler.protocol.bind<fb::login::handler::protocol::complete>();
    this->handler.protocol.bind<fb::login::handler::protocol::change_password>();
}

server::~server()
{ }

bool server::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

async::task<void> server::on_start()
{
    co_await fb::acceptor<session>::on_start();

    this->bind_timer<fb::login::handler::timer::heart_beat>(1s);
    this->handler.amqp.bind<fb::login::handler::amqp::shutdown>("fb.system");
}

async::task<void> server::update_status()
{
    try
    {
        auto world = fb::config<uint32_t>("world");
        std::ignore = co_await this->http.post("internal",
                                               "/server/heartbeat",
                                               internal_reqs::Heartbeat{world,
                                                                        internal::Service::Login,
                                                                        this->id(),
                                                                        this->name(),
                                                                        fb::config<std::string>("ip"),
                                                                        fb::config<uint16_t>("port")});
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to send heartbeat: {}", e.what());
    }
}

const fb::protocol::login::response::agreement& server::agreement() const
{
    return this->_agreement;
}

void server::assert_account(const std::string& id, const std::string& pw) const
{
    auto cp949     = CP949(id);
    auto name_size = cp949.length();

    if (name_size < fb::config<int>("name_size:min") || name_size > fb::config<int>("name_size:max"))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name must be full-hangul characters
    if (fb::config<bool>("allow_foreign_name") == false && assert_korean(cp949) == false)
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    if (fb::model::table::blocked_name.contains_substring(id))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    if (fb::model::table::blocked_word.contains_substring(id))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Read character's password
    if (pw.length() < fb::config<int>("pw_size:min") || pw.length() > fb::config<int>("pw_size:max"))
        throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));
}

async::task<void> server::on_accepted(fb::socket<session>& socket)
{
    auto data = std::make_shared<session>();
    socket.data(data);
    co_return;
}

async::task<bool> server::on_connected(fb::socket<session>& socket)
{
    co_return true;
}

async::task<bool> server::on_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

void server::on_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
}