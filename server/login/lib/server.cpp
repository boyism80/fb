#include <boost/asio/high_resolution_timer.hpp>
#include <fb/login/server.h>
#include <fb/login/handler.h>
#include <format>

using namespace fb::login;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<session>(io_context, "LOGIN", port),
    _redis(config<std::string>("redis:ip").c_str(), config<uint16_t>("redis:port"), config<uint32_t>("redis:pool"))
{
    for (auto& x : fb::config<>("forbidden"))
        this->_forbiddens.push_back(x.asString());

    // Register event handler
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

async::task<void> server::handle_start()
{
    co_await fb::acceptor<session>::handle_start();

    this->bind_timer(&server::handle_heart_beat, 1s);
    this->handler.amqp.bind("fb.system", &server::handle_amqp_shutdown);
}

async::task<void> server::handle_heart_beat()
{
    auto root    = Json::Value{};
    root["Name"] = this->name();
    root["IP"]   = fb::config<std::string>("ip");
    root["Port"] = fb::config<uint16_t>("port");
    auto writer  = Json::FastWriter{};
    auto output  = writer.write(root);

    this->_redis.command<void>(std::format("SET heart-beat:Login:{} {}", this->id(), output));
    this->_redis.command<void>(std::format("EXPIRE heart-beat:Login:{} 5", this->id()));
    co_return;
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

async::task<void> server::handle_amqp_shutdown(const internal_resp::Shutdown& response)
{
    this->exit();
    co_return;
}

void server::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
}