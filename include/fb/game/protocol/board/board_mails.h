#ifndef __PROTOCOL_GAME_MAILS_H__
#define __PROTOCOL_GAME_MAILS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class board_mails : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const MAIL_BUTTON_ENABLE button_flags;

public:
    board_mails(MAIL_BUTTON_ENABLE button_flags);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif