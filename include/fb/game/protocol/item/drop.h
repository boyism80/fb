#ifndef __PROTOCOL_GAME_DROP_H__
#define __PROTOCOL_GAME_DROP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class item_drop : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x08;

public:
#ifndef BOT
    uint8_t index;
    bool    all;
#else
    const uint8_t index;
    const bool    all;
#endif

public:
#ifndef BOT
    item_drop() = default;
#else
    item_drop(uint8_t index, bool all) :
        index(index),
        all(all)
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