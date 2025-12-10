#ifndef __PROTOCOL_GAME_BRIGHT_H__
#define __PROTOCOL_GAME_BRIGHT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class bright : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x20;

public:
#ifndef BOT
    const uint8_t value;
#else
    uint8_t value;
#endif

public:
#ifndef BOT
    bright(uint8_t value) :
        value(value)
    { }
#else
    bright() = default;
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