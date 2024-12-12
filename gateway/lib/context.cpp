#include <context.h>

using namespace fb::gateway;
using namespace fb::protocol::gateway;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<session>(context, "GATEWAY", port)
{
    // Register event handler
    this->bind(&context::handle_check_version);
    this->bind(&context::handle_entry_list);
}

context::~context()
{ }

async::task<void> context::load_entries()
{
    // Load gateway list
    auto& entrypoints = fb::config<>("entrypoints");
    for (auto i = entrypoints.begin(); i != entrypoints.end(); i++)
    {
        this->_entrypoints.push_back(entry(cp949((*i)["name"].asCString()),
                                           cp949((*i)["desc"].asCString()),
                                           (*i)["ip"].asCString(),
                                           (*i)["port"].asInt()));
    }

    auto writer = fb::stream_writer<big_endian>(this->_entry_stream_cache);
    co_await response::hosts(this->_entrypoints).serialize(writer);
    this->_entry_crc32_cache = this->_entry_stream_cache.crc();
}

fb::stream context::make_crt_stream(const fb::cryptor& crt)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    writer.write<uint8_t>(0x00); // cmd : 0x00
    writer.write<uint8_t>(0x00);
    writer.write<uint32_t>(this->_entry_crc32_cache);
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
    co_await socket.send(this->_connection_cache, false);

    fb::logger::info("{}님이 접속했습니다.", socket.IP());
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<session>& socket)
{
    fb::logger::info("{}님의 연결이 끊어졌습니다.", socket.IP());
    co_return false;
}

async::task<bool> context::handle_check_version(fb::socket<session>& socket, const request::assert_version& request)
{
    try
    {
        util::assert_client(request);

        auto crt = cryptor::generate();
        socket.crt(crt);

        co_await this->send(socket, response::crt(crt, this->_entry_crc32_cache), false);
        co_return true;
    }
    catch (std::exception& e)
    {
        co_return false;
    }
}

async::task<bool> context::handle_entry_list(fb::socket<session>& socket, const request::entry_list& request)
{
    switch (request.action)
    {
    case 0x00:
    {
        const auto& entry = this->_entrypoints[request.index];
        co_await this->transfer(socket, entry.ip, entry.port, fb::protocol::internal::Service::Gateway);
        co_return true;
    }

    case 0x01:
    {
        co_await this->send(socket, this->_entry_stream_cache);
        co_return true;
    }

    default:
        co_return false;
    }
}