#ifndef __PROTOCOL_GAME_UNKNOWN_63_H__
#define __PROTOCOL_GAME_UNKNOWN_63_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <array>
#include <string>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

struct unknown_63_entry
{
    std::string            name;
    uint8_t                flag = 0;
    uint16_t               a    = 0;
    std::array<uint8_t, 5> b{};
    uint16_t               c = 0;
    uint8_t                d = 0;
    uint32_t               e = 0;
    uint32_t               f = 0;
};

template <CLIENT_VERSION V>
class unknown_63 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x63;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t                       subtype = 2;
    uint8_t                       count   = 0;
    std::vector<unknown_63_entry> entries;
#else
    const uint8_t                       subtype;
    const uint8_t                       count;
    const std::vector<unknown_63_entry> entries;
#endif

public:
#ifdef BOT
    unknown_63() = default;
#else
    unknown_63(uint8_t subtype, uint8_t count, std::vector<unknown_63_entry> entries = {});
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
