#ifndef __PROTOCOL_GAME_CLICK_H__
#define __PROTOCOL_GAME_CLICK_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class click : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x43;

public:
#ifndef BOT
    uint32_t oid;
#else
    const uint32_t oid;
#endif

public:
#ifndef BOT
    click() = default;
#else
    click(uint32_t oid) :
        oid(oid)
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