#ifndef __PROTOCOL_GAME_EFFECT_H__
#define __PROTOCOL_GAME_EFFECT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class effect : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x29;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const fb::game::object& me;
    const uint8_t           value;
#else
    uint32_t oid;
    uint8_t  value;
#endif

public:
#ifndef BOT
    effect(const fb::game::object& me, uint8_t value);
#else
    effect() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using effect_v550 = effect<CLIENT_VERSION::v550>;
using effect_v565 = effect<CLIENT_VERSION::v565>;
using effect_v651 = effect<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
