#ifndef __PROTOCOL_GATEWAY_SERVER_LIST_H__
#define __PROTOCOL_GATEWAY_SERVER_LIST_H__

#include <fb/protocol/header.h>
#include <format>
#include <string_view>

namespace fb::protocol::gateway {

class endpoint
{
public:
    std::string name;
    std::string desc;
    uint32_t    ip   = 0;
    uint16_t    port = 0;

public:
    endpoint(std::string_view name, std::string_view desc, uint32_t ip, uint16_t port);
    endpoint(std::string_view name, std::string_view desc, std::string_view ip, uint16_t port);
    endpoint(const endpoint& right);
    ~endpoint() = default;
};

} // namespace fb::protocol::gateway

namespace fb::protocol::gateway::request {

class server_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x57;

public:
    uint8_t action;
    uint8_t index;

public:
#ifndef BOT
    server_list() = default;
#else
    server_list(uint8_t action, uint8_t index);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::request

namespace fb::protocol::gateway::response {

class server_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x56;

public:
#ifdef BOT
    std::vector<fb::protocol::gateway::endpoint> servers;
#else
    const std::vector<fb::protocol::gateway::endpoint>& servers;
#endif

public:
#ifdef BOT
    server_list() = default;
#else
    server_list(const std::vector<fb::protocol::gateway::endpoint>& servers);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::response

#endif
