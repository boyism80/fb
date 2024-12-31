#ifndef __PROTOCOL_GAME_MAIL_H__
#define __PROTOCOL_GAME_MAIL_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class board_mail : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const uint8_t button_flag;

public:
    board_mail(uint8_t button_flag);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif