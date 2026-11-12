#include <fb/gateway/protocol/server_list.h>

#include <cstdint>
#include <format>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <fb/winsock.h>
#else
#include <arpa/inet.h>
#endif

using namespace fb::protocol::gateway;

endpoint::endpoint(std::string_view name, std::string_view desc, uint32_t ip, uint16_t port) :
    name(std::string(name)),
    desc(std::string(desc)),
    ip(ip),
    port(port)
{ }

endpoint::endpoint(std::string_view name, std::string_view desc, std::string_view ip, uint16_t port) :
    endpoint(name, desc, inet_addr(std::string(ip).c_str()), port)
{ }

endpoint::endpoint(const endpoint& right) :
    endpoint(right.name, right.desc, right.ip, right.port)
{ }

namespace fb::protocol::gateway::request {

#ifndef BOT
template <CLIENT_VERSION V>
void server_list<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->action = reader.read<uint8_t>();
    if (action == 0x00)
        this->index = reader.read<uint8_t>();
}
#else
template <CLIENT_VERSION V>
server_list<V>::server_list(uint8_t action, uint8_t index) :
    action(action),
    index(index)
{ }

template <CLIENT_VERSION V>
void server_list<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->action);

    if (this->action == 0x00)
        writer.write<uint8_t>(this->index);
}
#endif

template class server_list<CLIENT_VERSION::v550>;
template class server_list<CLIENT_VERSION::v565>;
template class server_list<CLIENT_VERSION::v651>;

} // namespace fb::protocol::gateway::request

namespace fb::protocol::gateway::response {

#ifndef BOT
server_list::server_list(const std::vector<fb::protocol::gateway::endpoint>& servers) :
    servers(servers)
{ }
#endif

#ifndef BOT
void server_list::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    auto formats = fb::stream();
    {
        auto writer = fb::stream_writer<big_endian>(formats);
        writer.write<uint8_t>((uint8_t)this->servers.size());
        for (uint32_t i = 0; i < this->servers.size(); i++)
        {
            auto gateway = this->servers.at(i);
            auto buffer  = std::format("{};{}", gateway.name, gateway.desc);

            writer.write<uint8_t>(i);
            writer.write<uint32_t>(gateway.ip);
            writer.write<uint16_t>(gateway.port);
            writer.write(buffer.c_str(), buffer.size() + 1);
        }
    }

    auto compressed = formats.compress();

    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(compressed.size());
    writer.write(compressed.data(), compressed.size());
    writer.write<uint8_t>(0);
}
#else
void server_list::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    auto size       = reader.read<uint16_t>();
    auto compressed = std::vector<uint8_t>(size);
    reader.read(compressed.data(), size);
    reader.read<uint8_t>();

    auto formats = fb::stream(compressed.data(), size).decompress();
    auto entries = fb::stream_reader<big_endian>(formats);
    auto count   = entries.read<uint8_t>();
    this->servers.clear();
    for (uint8_t i = 0; i < count && entries.readable_size() > 0; i++)
    {
        entries.read<uint8_t>();
        auto ip   = entries.read<uint32_t>();
        auto port = entries.read<uint16_t>();
        auto text = std::string();
        while (entries.readable_size() > 0)
        {
            auto value = entries.read<uint8_t>();
            if (value == 0)
                break;

            text.push_back(static_cast<char>(value));
        }

        auto split = text.find(';');
        auto name  = split == std::string::npos ? text : text.substr(0, split);
        auto desc  = split == std::string::npos ? std::string() : text.substr(split + 1);
        this->servers.emplace_back(name, desc, ip, port);
    }
}
#endif

} // namespace fb::protocol::gateway::response
