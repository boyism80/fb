#ifndef __PROTOCOL_GAME_MESSAGE_H__
#define __PROTOCOL_GAME_MESSAGE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class message : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x0A;

public:
#ifdef BOT
    std::string  text;
    MESSAGE_TYPE type;
#else
    const std::string  text;
    const MESSAGE_TYPE type;
#endif

public:
#ifdef BOT
    message() = default;
#else
    message(const std::string& text, MESSAGE_TYPE type);
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