#ifndef __PROTOCOL_GAME_UNKNOWN_70_H__
#define __PROTOCOL_GAME_UNKNOWN_70_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * Single marker of the S2C 0x70 web world map list: position and CP949 label.
 */
struct unknown_70_entry
{
    uint16_t    x = 0;
    uint16_t    y = 0;
    std::string name;
};

/**
 * S2C 0x70 — Nexon web world map window (CharStats NEW UI only, window 620), since 6.51.
 * Wire: [0x70][count u8] then count entries of [x u16][y u16][name_len u8][name CP949].
 */
template <CLIENT_VERSION V>
class unknown_70 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x70;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    std::vector<unknown_70_entry> entries;
#else
    const std::vector<unknown_70_entry> entries;
#endif

public:
#ifdef BOT
    unknown_70() = default;
#else
    explicit unknown_70(const std::vector<unknown_70_entry>& entries) :
        entries(entries)
    { }
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
