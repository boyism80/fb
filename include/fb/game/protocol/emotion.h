#ifndef __PROTOCOL_GAME_EMOTION_H__
#define __PROTOCOL_GAME_EMOTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class emotion : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1D;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    uint8_t value;
#else
    const uint8_t value;
#endif

public:
#ifndef BOT
    emotion() = default;
#else
    emotion(uint8_t value);
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