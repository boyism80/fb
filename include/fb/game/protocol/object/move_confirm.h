#ifndef __PROTOCOL_GAME_MOVE_CONFIRM_H__
#define __PROTOCOL_GAME_MOVE_CONFIRM_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class move_confirm : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0B;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const DIRECTION                  direction;
    const fb::model::point<uint16_t> position;
    const fb::model::point<uint16_t> viewport;
    const uint8_t                    walk_queue_slot;
#else
    DIRECTION                  direction;
    fb::model::point<uint16_t> position;
    fb::model::point<uint16_t> viewport;
    uint8_t                    walk_queue_slot = 0;
#endif

public:
#ifndef BOT
    move_confirm(DIRECTION                         direction,
                 const fb::model::point<uint16_t>& position,
                 const fb::model::point<uint16_t>& viewport,
                 uint8_t                           walk_queue_slot);
    move_confirm(const fb::game::object&           object,
                 const fb::model::point<uint16_t>& position,
                 const fb::model::point<uint16_t>& viewport,
                 uint8_t                           walk_queue_slot);
#else
    move_confirm() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using move_confirm_v550 = move_confirm<CLIENT_VERSION::v550>;
using move_confirm_v565 = move_confirm<CLIENT_VERSION::v565>;
using move_confirm_v651 = move_confirm<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
