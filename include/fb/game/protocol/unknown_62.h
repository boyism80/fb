#ifndef __PROTOCOL_GAME_UNKNOWN_62_H__
#define __PROTOCOL_GAME_UNKNOWN_62_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x62 — NEW UI window open (client sub_491C40, UI 880 on NEW / 620 on OLD), since 6.51.
 * Body is empty. Shares the opcode with the gateway C2S connection_ack but not the meaning.
 */
template <CLIENT_VERSION V>
class unknown_62 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x62;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
    unknown_62() = default;

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
