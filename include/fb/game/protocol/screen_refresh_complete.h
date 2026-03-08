#ifndef __PROTOCOL_GAME_SCREEN_REFRESH_COMPLETE_H__
#define __PROTOCOL_GAME_SCREEN_REFRESH_COMPLETE_H__

#include <fb/protocol/header.h>

namespace fb::protocol::game::response {

/**
 * Response sent after screen refresh (cmd 0x38) to signal refresh completion.
 * Corresponds to roseserver raw packet: 0xAA 0x00 0x02 0x22 0x00.
 * In FB, serialize writes only payload: cmd (0x22) + one byte (0x00);
 * magic and size are added by encryption wrap layer.
 */
class screen_refresh_complete : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x22;

public:
    screen_refresh_complete() = default;

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
