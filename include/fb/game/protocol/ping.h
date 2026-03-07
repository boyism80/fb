#ifndef __PROTOCOL_GAME_PING_H__
#define __PROTOCOL_GAME_PING_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * Server-to-client ping (0x68).
 * Payload: 4-byte big-endian uint32_t (token).
 */
class ping : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x68;

public:
#ifndef BOT
    const uint32_t value;
#else
    uint32_t value;
#endif

public:
#ifndef BOT
    explicit ping(uint32_t value) :
        value(value)
    { }
#else
    ping() = default;
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
