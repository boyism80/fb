#ifndef __PROTOCOL_GAME_MAIL_H__
#define __PROTOCOL_GAME_MAIL_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

using namespace fb::model::enum_value;
using namespace fb::protocol::internal;

namespace fb::protocol::game::response {

class bulletin_mail : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const Mail&              mail;
    const MAIL_BUTTON_ENABLE flag;

public:
    bulletin_mail(const Mail& mail, MAIL_BUTTON_ENABLE flag);

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif