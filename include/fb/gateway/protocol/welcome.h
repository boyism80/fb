#ifndef __PROTOCOL_GATEWAY_WELCOME_H__
#define __PROTOCOL_GATEWAY_WELCOME_H__

#include <fb/protocol/header.h>

namespace fb::protocol::gateway::response {

class welcome : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x7E;

public:
    welcome() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::gateway::response

#endif