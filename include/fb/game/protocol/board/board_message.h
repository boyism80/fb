#ifndef __PROTOCOL_GAME_BOARD_MESSAGE_H__
#define __PROTOCOL_GAME_BOARD_MESSAGE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class board_message : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const std::string text;
    const bool        success;
    const bool        mail;

public:
    board_message(const std::string& text, bool success, bool mail = false);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif