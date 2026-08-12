#ifndef __PROTOCOL_GAME_UNKNOWN_4F_H__
#define __PROTOCOL_GAME_UNKNOWN_4F_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x4F — opens a custom UI window (client sub_48C3E0, 6.51 window size 2116), since 5.65.
 * Body is empty; the client answers with C2S 0x54 phase 5 once the window is built.
 */
template <CLIENT_VERSION V>
class unknown_4f : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
    unknown_4f() = default;

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
