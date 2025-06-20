#include <fb/gateway/context.h>

using namespace fb::gateway;
using namespace fb::protocol::gateway;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<session>(context, "GATEWAY", port)
{
    // Register event handler
    this->handler.protocol.bind(&context::handle_check_version);
    this->handler.protocol.bind(&context::handle_entry_list);
}

context::~context()
{ }

async::task<void> context::load_entries()
{
    // Load gateway list
    auto& entrypoints = fb::config<>("entrypoints");
    for (auto i = entrypoints.begin(); i != entrypoints.end(); i++)
    {
        this->_entrypoints.push_back(endpoint(cp949((*i)["name"].asCString()),
                                              cp949((*i)["desc"].asCString()),
                                              this->ipv4((*i)["ip"].asString()),
                                              (*i)["port"].asInt()));
    }

    auto writer = fb::stream_writer<big_endian>(this->_endpoint_bytes);
    co_await fb::protocol::gateway::response::endpoint(this->_entrypoints).serialize(writer);
    this->_endpoint_crc = this->_endpoint_bytes.crc();
}

fb::stream context::make_crt_stream(const fb::crypto& crt)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    writer.write<uint8_t>(0x00); // cmd : 0x00
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->_endpoint_crc);
    writer.write<uint8_t>(crt.type());
    writer.write<uint8_t>(0x09);
    writer.write(crt.key(), 0x09);
    writer.write<uint8_t>(0x00);

    return stream;
}

bool context::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x00:
        return false;

    default:
        return true;
    }
}

async::task<void> context::handle_start()
{
    static constexpr const char* message = "CONNECTED SERVER\n";

    this->handler.amqp.bind("fb.system", &context::handle_amqp_shutdown);

    auto writer = fb::stream_writer<big_endian>(this->_connection_cache);
    writer.write<uint8_t>(0x7E);
    writer.write<uint8_t>(0x1B);
    writer.write((const void*)message, strlen(message));
    co_await this->load_entries();
}

session* context::handle_accepted(fb::socket<session>& socket)
{
    auto uptr = std::make_unique<session>();
    auto ptr  = uptr.get();
    this->_sessions.push_back(std::move(uptr));
    return ptr;
}

async::task<bool> context::handle_connected(fb::socket<session>& socket)
{
    socket.send(this->_connection_cache, false);

    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

async::task<void> fb::gateway::context::handle_amqp_shutdown(const internal_resp::Shutdown& response)
{
    this->exit();
    co_return;
}

async::task<bool> context::handle_check_version(fb::socket<session>&                           socket,
                                                const fb::protocol::gateway::request::version& request)
{
    try
    {
        util::assert_client(request);

        auto crt = crypto::generate();
        socket.crt(crt);

        this->send(socket, response::crypto(crt, this->_endpoint_crc), false);
        co_return true;
    }
    catch (std::exception&)
    {
        co_return false;
    }
}

async::task<bool> context::handle_entry_list(fb::socket<session>&                            socket,
                                             const fb::protocol::gateway::request::endpoint& request)
{
    switch (request.action)
    {
    case 0x00:
    {
        const auto& entry = this->_entrypoints[request.index];
        std::ignore       = this->transfer(socket, entry.ip, entry.port, fb::protocol::internal::Service::Gateway);
        co_return true;
    }

    case 0x01:
    {
        this->send(socket, this->_endpoint_bytes);
        co_return true;
    }

    default:
        co_return false;
    }
}

void context::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
}