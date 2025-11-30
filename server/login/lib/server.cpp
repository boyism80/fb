#include <boost/asio/high_resolution_timer.hpp>
#include <fb/login/server.h>
#include <fb/login/handler.h>
#include <fb/log_collector.h>
#include <format>

using namespace fb::login;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<session>(io_context, "LOGIN", port)
{
    for (auto& x : fb::config<>("forbidden"))
        this->_forbiddens.push_back(x.asString());

    // Register event handler
    this->handler.protocol.bind<fb::login::handler::protocol::login>();
    this->handler.protocol.bind<fb::login::handler::protocol::agreement>();
    this->handler.protocol.bind<fb::login::handler::protocol::create_account>();
    this->handler.protocol.bind<fb::login::handler::protocol::complete>();
    this->handler.protocol.bind<fb::login::handler::protocol::change_password>();

    // Initialize log collector
    try
    {
        auto log_server_url = std::format("http://{}:{}", fb::config<std::string>("log:ip"), fb::config<uint16_t>("log:port"));
        auto server_id      = std::to_string(this->id());
        auto server_name    = this->name();
        this->log           = std::make_unique<fb::log_collector>(this->http, server_id, server_name, log_server_url, 1000, 100);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("Failed to initialize log collector: {}", e.what());
    }
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

async::task<void> server::handle_start()
{
    co_await fb::acceptor<session>::handle_start();

    this->bind_timer<fb::login::handler::timer::heart_beat>(1s);
    this->bind_timer<fb::login::handler::timer::log_flush>(5s);
    this->handler.amqp.bind<fb::login::handler::amqp::shutdown>("fb.system");
}

async::task<void> server::update_status()
{
    try
    {
        co_await this->http.post("internal",
                                 "/server/heartbeat",
                                 Heartbeat{internal::Service::Login, this->id(), this->name(), fb::config<std::string>("ip"), fb::config<uint16_t>("port")});
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

bool server::is_forbidden(const std::string& str) const
{
    return std::any_of(this->_forbiddens.cbegin(), this->_forbiddens.cend(), [str](const auto& x) {
        return x == str;
    });
}

void server::assert_account(const std::string& id, const std::string& pw) const
{
    auto cp949     = CP949(id);
    auto name_size = cp949.length();

    if (name_size < fb::config<int>("name_size:min") || name_size > fb::config<int>("name_size:max"))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name must be full-hangul characters
    if (fb::config<bool>("allow other language") == false && assert_korean(cp949) == false)
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Name cannot contains subcharacters in forbidden list
    if (this->is_forbidden(id))
        throw id_exception(_TEXT(MESSAGE_ACCOUNT_INVALID_NAME));

    // Read character's password
    if (pw.length() < fb::config<int>("pw_size:min") || pw.length() > fb::config<int>("pw_size:max"))
        throw pw_exception(_TEXT(MESSAGE_ACCOUNT_PASSWORD_SIZE));
}

std::shared_ptr<session> server::handle_accepted(fb::socket<session>& socket)
{
    return std::make_shared<session>();
}

async::task<bool> server::handle_connected(fb::socket<session>& socket)
{
    co_return true;
}

async::task<bool> server::handle_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

void server::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
}