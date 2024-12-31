#ifndef __PROTOCOL_GATEWAY_ENTRY_LIST_H__
#define __PROTOCOL_GATEWAY_ENTRY_LIST_H__

#include <fb/protocol/header.h>
#include <format>

namespace fb::protocol::gateway {

class endpoint
{
public:
    std::string name;
    std::string desc;
    uint32_t    ip   = 0;
    uint16_t    port = 0;

public:
    endpoint(const std::string& name, const std::string& desc, uint32_t ip, uint16_t port);
    endpoint(const std::string& name, const std::string& desc, const std::string& ip, uint16_t port);
    endpoint(const endpoint& right);
    ~endpoint() = default;
};

} // namespace fb::protocol::gateway

namespace fb::protocol::gateway::request {

class endpoint : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x57;

public:
    uint8_t action;
    uint8_t index;

public:
#ifndef BOT
    endpoint() = default;
#else
    endpoint(uint8_t action, uint8_t index);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::request

namespace fb::protocol::gateway::response {

class endpoint : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x56;

public:
#ifdef BOT
    std::vector<fb::protocol::gateway::endpoint> entries;
#else
    const std::vector<fb::protocol::gateway::endpoint>& entries;
#endif

public:
#ifdef BOT
    endpoint() = default;
#else
    endpoint(const std::vector<fb::protocol::gateway::endpoint>& entries);
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::gateway::response

#endif