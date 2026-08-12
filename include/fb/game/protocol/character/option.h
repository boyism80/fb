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
#endif

public:
#ifdef BOT
    option() = default;
#else
    option(const fb::game::character& ch) :
        ch(ch)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        header::serialize(writer);
        writer.write<uint8_t>(opcode);
        writer.write<uint8_t>(this->ch.option(OPTION::WEATHER_EFFECT)); // weather
        writer.write<uint8_t>(this->ch.option(OPTION::MAGIC_EFFECT));   // magic effect
        writer.write<uint8_t>(this->ch.option(OPTION::NEWS));           // news
        writer.write<uint8_t>(this->ch.option(OPTION::FAST_MOVE));      // fast move
        writer.write<uint8_t>(this->ch.option(OPTION::EFFECT_SOUND));   // effect sound
        // v550/v565: 5 flags. v651 appends SELFLOOK.
        if constexpr (V == CLIENT_VERSION::v651)
            writer.write<uint8_t>(0); // SELFLOOK sheet toggle (off)
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        header::deserialize(reader);
        this->weather_effect = reader.read<uint8_t>();
        this->magic_effect   = reader.read<uint8_t>();
        this->news           = reader.read<uint8_t>();
        this->fast_move      = reader.read<uint8_t>();
        this->effect_sound   = reader.read<uint8_t>();
        if constexpr (V == CLIENT_VERSION::v651)
            this->selflook = reader.read<uint8_t>();
    }
#endif
};

using option_v550 = option<CLIENT_VERSION::v550>;
using option_v565 = option<CLIENT_VERSION::v565>;
using option_v651 = option<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
