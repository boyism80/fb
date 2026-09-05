#ifndef __PROTOCOL_GAME_OBJECT_DIRECTION_H__
#define __PROTOCOL_GAME_OBJECT_DIRECTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class direction : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x11;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    DIRECTION value;
#else
    const DIRECTION value;
#endif

public:
#ifndef BOT
    direction() = default;
#else
    direction(DIRECTION value);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class direction : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x11;

public:
#ifndef BOT
    const uint32_t  oid;
    const DIRECTION value;
#else
    uint32_t  oid;
    DIRECTION value;
#endif

public:
#ifndef BOT
    direction(const fb::game::object& object);
    direction(uint32_t oid, DIRECTION value);
#else
    direction() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif