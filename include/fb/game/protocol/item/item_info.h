#ifndef __PROTOCOL_GAME_INFO_H__
#define __PROTOCOL_GAME_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_info : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x66;

public:
#ifndef BOT
    uint16_t position;
    uint8_t  slot;
#else
    const uint16_t position = 0;
    const uint8_t  slot     = 0;
#endif

public:
#ifndef BOT
    item_info() = default;
#else
    item_info(uint16_t position, uint8_t slot) :
        position(position),
        slot(slot)
    { }
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif