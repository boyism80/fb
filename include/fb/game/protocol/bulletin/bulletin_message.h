#ifndef __PROTOCOL_GAME_BULLETIN_MESSAGE_H__
#define __PROTOCOL_GAME_BULLETIN_MESSAGE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class bulletin_message : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x31;

public:
#ifndef BOT
    const std::string text;
    const bool        success;
    const bool        unknown;
#else
    std::string text;
    bool        success;
    bool        unknown;
#endif

public:
#ifndef BOT
    bulletin_message(std::string_view text, bool success, bool unknown = false);
#else
    bulletin_message() = default;
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