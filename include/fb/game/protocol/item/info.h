#ifndef __PROTOCOL_GAME_INFO_H__
#define __PROTOCOL_GAME_INFO_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x66;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint16_t position = 0;
    uint8_t  filler0  = 0;
    uint8_t  filler1  = 0;
    uint8_t  filler2  = 0;
    uint8_t  slot     = 0;
#else
    const uint16_t position = 0;
    const uint8_t  filler0  = 0;
    const uint8_t  filler1  = 1;
    const uint8_t  filler2  = 1;
    const uint8_t  slot     = 0;
#endif

public:
#ifndef BOT
    item_info() = default;
#else
    item_info(uint16_t position, uint8_t slot);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
