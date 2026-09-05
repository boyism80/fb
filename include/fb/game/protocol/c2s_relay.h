#ifndef __PROTOCOL_GAME_C2S_RELAY_H__
#define __PROTOCOL_GAME_C2S_RELAY_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class c2s_relay : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4B;

public:
#ifndef BOT
    const std::string payload;
#else
    std::string payload;
#endif

public:
#ifndef BOT
    explicit c2s_relay(std::string_view payload);
#else
    c2s_relay() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
