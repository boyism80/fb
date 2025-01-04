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
    const uint8_t     type;
    const std::string address;
    const std::string message;

public:
    web(uint8_t type, std::string address, std::string message);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif