#ifndef __PROTOCOL_GAME_SWAP_H__
#define __PROTOCOL_GAME_SWAP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class swap : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x30;

public:
#ifndef BOT
    SWAP_TYPE type;
    uint8_t   src;
    uint8_t   dst;
#else
    const SWAP_TYPE type;
    const uint8_t   src;
    const uint8_t   dst;
#endif

public:
#ifndef BOT
    swap() = default;
#else
    swap(const SWAP_TYPE type, const uint8_t src, const uint8_t dst) :
        type(type),
        src(src),
        dst(dst)
    { }
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