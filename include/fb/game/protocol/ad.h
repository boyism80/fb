#ifndef __PROTOCOL_GAME_AD_H__
#define __PROTOCOL_GAME_AD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class ad : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x5B;

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
    ad(uint32_t width, uint32_t height, std::string url, uint8_t time) :
        width(width),
        height(height),
        url(url),
        time(time)
    { }
#else
    ad() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif