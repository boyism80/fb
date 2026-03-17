#include <fb/gateway/server.h>
#include <fb/gateway/handler.h>
#include <fb/gateway/protocol.h>
#include <fb/log_collector.h>
#include <format>

using namespace fb::gateway;
using namespace fb::protocol::gateway;
using namespace std::chrono_literals;

namespace internal_reqs = fb::protocol::internal::request;

server::server(boost::asio::io_context& io_context, uint16_t port) :
    fb::acceptor<session>(io_context, "GATEWAY", port, fb::config<uint32_t>("http:max_concurrent", 500)),
    log(fb::config<std::string>("amqp:log:ip"),
        fb::config<uint16_t>("amqp:log:port"),
        fb::config<std::string>("amqp:log:uid"),
        fb::config<std::string>("amqp:log:pwd"),
        std::to_string(fb::config<uint32_t>("id")),
        fb::config<std::string>("name"),
        0) // Gateway is unified (world = 0)
{
    this->handler.protocol.bind<fb::gateway::handler::protocol::version>();
    this->handler.protocol.bind<fb::gateway::handler::protocol::server_list>();
    this->handler.protocol.bind<fb::gateway::handler::protocol::connection_ack>();
}

server::~server()
{ }

const std::vector<endpoint>& server::entrypoints() const
{
    return this->_entrypoints;
}

const fb::stream& server::endpoint_bytes() const
{
    return this->_endpoint_bytes;
}

uint32_t server::endpoint_crc() const
{
    return this->_endpoint_crc;
}

async::task<void> server::load_entries()
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
    co_await fb::protocol::gateway::response::server_list(this->_entrypoints).serialize(writer);
    this->_endpoint_crc = this->_endpoint_bytes.crc();
}

fb::stream server::make_crt_stream(const fb::encryption& encryption)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    writer.write<uint8_t>(0x00); // opcode : 0x00
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->_endpoint_crc);
    writer.write<uint8_t>(encryption.pattern());
    writer.write<uint8_t>(fb::encryption::KEY_SIZE);
    writer.write(encryption.iv(), fb::encryption::KEY_SIZE);
    writer.write<uint8_t>(0x00);

    return stream;
}

bool server::decrypt_policy(uint8_t opcode) const
{
    switch (opcode)
    {
    case fb::protocol::gateway::request::version::opcode:
    case fb::protocol::gateway::request::connection_ack::opcode:
        return false;

    default:
        return true;
    }
}

async::task<void> server::on_start()
{
    static constexpr const char* message = "CONNECTED SERVER\n";

    this->bind_timer<fb::gateway::handler::timer::heart_beat>(1s);
    this->handler.amqp.bind<fb::gateway::handler::amqp::shutdown>("fb.global"); // Shutdown: all servers

    auto writer = fb::stream_writer<big_endian>(this->_connection_welcome);
    writer.write<uint8_t>(0x7E);
    writer.write<uint8_t>(0x1B);
    writer.write((const void*)message, strlen(message));
    co_await this->load_entries();
}

async::task<void> server::on_accepted(fb::socket<session>& socket)
{
    auto data = std::make_shared<session>();
    socket.data(data);
    co_return;
}

async::task<bool> server::on_connected(fb::socket<session>& socket)
{
    socket.send(this->_connection_welcome, false);

    co_return true;
}

async::task<bool> server::on_disconnected(fb::socket<session>& socket)
{
    co_return false;
}

void server::on_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.global"); // Shutdown: all servers
}

async::task<void> server::update_status()
{
    try
    {
        // Gateway is a global service that doesn't belong to any specific world
        // Use 0 as the world identifier for gateway heartbeat (0 represents unified)
        std::ignore = co_await this->http.post("internal",
                                               "/server/heartbeat",
                                               internal_reqs::Heartbeat{0,
                                                                        internal::Service::Gateway,
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
