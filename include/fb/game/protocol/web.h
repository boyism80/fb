#ifndef __PROTOCOL_GAME_WEB_H__
#define __PROTOCOL_GAME_WEB_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class web : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x66;

public:
#ifndef BOT
    const uint8_t     type;
    const std::string address;
    const std::string message;
#else
    uint8_t     type;
    std::string address;
    std::string message;
#endif

public:
#ifndef BOT
    web(uint8_t type, std::string address, std::string message) :
        type(type),
        address(address),
        message(message)
    { }
#else
    web() = default;
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