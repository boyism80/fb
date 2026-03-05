#ifndef __PROTOCOL_GAME_UNKNOWN_6A_H__
#define __PROTOCOL_GAME_UNKNOWN_6A_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class unknown_6A : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x6A;

public:
#ifndef BOT
    const uint8_t value;
#else
    uint8_t value;
#endif

public:
#ifndef BOT
    explicit unknown_6A(uint8_t value) :
        value(value)
    { }
#else
    unknown_6A() = default;
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
