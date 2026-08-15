#ifndef __PROTOCOL_GAME_OPTION_H__
#define __PROTOCOL_GAME_OPTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class option : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x23;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    bool weather_effect = false;
    bool magic_effect   = false;
    bool news           = false;
    bool fast_move      = false;
    bool effect_sound   = false;
    bool selflook       = false;
#else
    const fb::game::character& ch;
    uint8_t                    unknown_selflook = 0;
#endif

public:
#ifdef BOT
    option() = default;
#else
    option(const fb::game::character& ch);
    option(const fb::game::character& ch, uint8_t selflook);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <>
void option<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void option<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using option_v550 = option<CLIENT_VERSION::v550>;
using option_v565 = option<CLIENT_VERSION::v565>;
using option_v651 = option<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
