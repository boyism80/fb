#ifndef __PROTOCOL_LOGIN_MESSAGE_H__
#define __PROTOCOL_LOGIN_MESSAGE_H__

#include <fb/protocol/header.h>
#include <string_view>

namespace fb::protocol::login::response {

class message : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x02;

public:
#ifdef BOT
    std::string text;
    uint8_t     type;
#else
    const std::string text;
    const uint8_t     type;
#endif

public:
#ifdef BOT
    message() = default;
#else
    message(std::string_view text, uint8_t type);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::response

#endif
