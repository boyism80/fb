#ifndef __PROTOCOL_GAME_AD_H__
#define __PROTOCOL_GAME_AD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class ad : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x5B;

public:
    const uint32_t    width;
    const uint32_t    height;
    const std::string url;
    const uint8_t     time;

public:
    ad(uint32_t width, uint32_t height, std::string url, uint8_t time);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif