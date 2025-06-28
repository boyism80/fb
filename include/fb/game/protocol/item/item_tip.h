#ifndef __PROTOCOL_GAME_TIP_H__
#define __PROTOCOL_GAME_TIP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class item_tip : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x59;

public:
#ifndef BOT
    const uint16_t    position;
    const std::string message;
#else
    uint16_t    position;
    std::string message;
#endif

public:
#ifndef BOT
    item_tip(uint16_t position, const std::string& message) :
        position(position),
        message(message)
    { }
#else
    item_tip() = default;
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