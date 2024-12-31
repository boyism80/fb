#ifndef __PROTOCOL_GAME_BUNDLE_H__
#define __PROTOCOL_GAME_BUNDLE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class trade_bundle : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x42;

public:
    trade_bundle() = default;

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif