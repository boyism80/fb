#ifndef __PROTOCOL_GATEWAY_CONNECTION_ACK_H__
#define __PROTOCOL_GATEWAY_CONNECTION_ACK_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <string>

namespace fb::protocol::gateway::request {

/**
 * Client acknowledgment of the gateway connection welcome (handshake).
 * Sent by the client immediately after receiving the welcome packet (0x7E, 0x1B, "CONNECTED SERVER\n").
 * Payload: null-terminated client identifier string (e.g. "baram"), up to 6 bytes.
 */
template <CLIENT_VERSION V>
class connection_ack : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x62;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    std::string client_name;

public:
    connection_ack() = default;

public:
#ifndef BOT
    void deserialize(fb::stream_reader<big_endian>& reader) override;
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::gateway::request

#endif
