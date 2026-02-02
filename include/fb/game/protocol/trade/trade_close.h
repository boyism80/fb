#ifndef __PROTOCOL_GAME_CLOSE_H__
#define __PROTOCOL_GAME_CLOSE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class trade_close : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x42;

public:
#ifndef BOT
    const std::string message;
#else
    std::string message;
#endif

public:
#ifndef BOT
    trade_close(std::string_view message);
#else
    trade_close() = default;
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