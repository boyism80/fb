#ifndef __PROTOCOL_GAME_UNKNOWN_58_H__
#define __PROTOCOL_GAME_UNKNOWN_58_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x58 — bulk notice text window (CharStats NEW UI only, window 65788), since 6.51.
 * Wire: [0x58][flag u8][len u16 BE][text CP949]. flag 0 destroys the window.
 */
template <CLIENT_VERSION V>
class unknown_58 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x58;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t     flag = 0;
    std::string text;
#else
    const uint8_t     flag;
    const std::string text;
#endif

public:
#ifdef BOT
    unknown_58() = default;
#else
    unknown_58(uint8_t flag, const std::string& text) :
        flag(flag),
        text(text)
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
