#ifndef __PROTOCOL_LOGIN_KEEP_ALIVE_H__
#define __PROTOCOL_LOGIN_KEEP_ALIVE_H__

#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <fb/protocol/header.h>

#include <cstdint>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

// The client login scene sends this every 20 s with no payload.
template <CLIENT_VERSION V>
class keep_alive : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x71;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    keep_alive() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif
