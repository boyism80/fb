#ifndef __PROTOCOL_GAME_BUNDLE_H__
#define __PROTOCOL_GAME_BUNDLE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class trade_bundle : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;

public:
    trade_bundle() = default;

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif