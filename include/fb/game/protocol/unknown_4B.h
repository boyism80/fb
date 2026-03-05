#ifndef __PROTOCOL_GAME_UNKNOWN_4B_H__
#define __PROTOCOL_GAME_UNKNOWN_4B_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class unknown_4B : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x4B;

public:
#ifndef BOT
    const std::string payload;
#else
    std::string payload;
#endif

public:
#ifndef BOT
    explicit unknown_4B(std::string_view payload) :
        payload(std::string(payload))
    { }
#else
    unknown_4B() = default;
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
