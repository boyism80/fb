#ifndef __PROTOCOL_GAME_EMOTION_H__
#define __PROTOCOL_GAME_EMOTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class emotion : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x1D;

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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif