#ifndef __PROTOCOL_GAME_CLICK_H__
#define __PROTOCOL_GAME_CLICK_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class click : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x43;
    FB_PROTOCOL_VERSION_TAGS(V);

    // Client click kind: 1 = object/F-key, 3 = map coordinate (server ignores).
    static constexpr uint8_t FLAG_OBJECT = 1;
    static constexpr uint8_t FLAG_COORD  = 3;

public:
#ifndef BOT
    uint8_t  flag = 0;
    uint32_t oid  = 0;
#else
    const uint8_t  flag;
    const uint32_t oid;
#endif

public:
#ifndef BOT
    click() = default;
#else
    click(uint32_t oid, uint8_t flag = FLAG_OBJECT) :
        flag(flag),
        oid(oid)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
