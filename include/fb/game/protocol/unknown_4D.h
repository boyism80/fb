#ifndef __PROTOCOL_GAME_UNKNOWN_4D_H__
#define __PROTOCOL_GAME_UNKNOWN_4D_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <array>
#include <string>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * Unknown packet 0x4D (UserInfo).
 * Payload: type (1 byte). When type==2 only: exactly 8 strings, each len(1) + data (EUC-KR).
 * Client reads type then, if type==2, reads 8 length-prefixed strings; otherwise no more bytes.
 * Sending type!=2 avoids string/dropdown parsing and is safe; type==2 with non-matching
 * dropdown values can cause client crash (see PACKET_0x4D_0x68_STRUCTURE_ANALYSIS.md).
 */
class unknown_4D : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x4D;

public:
#ifndef BOT
    const uint8_t                    type;
    const std::array<std::string, 8> strings;
#else
    uint8_t                    type;
    std::array<std::string, 8> strings;
#endif

public:
#ifndef BOT
    explicit unknown_4D(uint8_t type, const std::array<std::string, 8>& strings) :
        type(type),
        strings(strings)
    { }
#else
    unknown_4D() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
