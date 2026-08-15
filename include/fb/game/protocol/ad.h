#ifndef __PROTOCOL_GAME_AD_H__
#define __PROTOCOL_GAME_AD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class ad : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x5B;

public:
#ifndef BOT
    const uint32_t    width;
    const uint32_t    height;
    const std::string url;
    const uint8_t     time;
#else
    uint32_t    width;
    uint32_t    height;
    std::string url;
    uint8_t     time;
#endif

public:
#ifndef BOT
    ad(uint32_t width, uint32_t height, std::string url, uint8_t time);
#else
    ad() = default;
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