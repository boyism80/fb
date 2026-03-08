#ifndef __PROTOCOL_GAME_FRIENDS_SYNC_H__
#define __PROTOCOL_GAME_FRIENDS_SYNC_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * Server tells the client whether to synchronize the friend list with the server.
 * When enabled is non-zero, the client sets g_friends_sync_enabled and sends
 * the friends list (request::friends, 0x77) to the server; when zero, sync is disabled.
 */
class friends_sync : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x6A;

public:
#ifndef BOT
    /** 0 = disable sync, non-zero = enable sync (client sends friends list to server). */
    const uint8_t enabled;
#else
    uint8_t enabled;
#endif

public:
#ifndef BOT
    explicit friends_sync(uint8_t enabled) :
        enabled(enabled)
    { }
#else
    friends_sync() = default;
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
