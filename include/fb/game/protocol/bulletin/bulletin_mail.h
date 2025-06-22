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
#ifndef BOT
    const Mail&              mail;
    const MAIL_BUTTON_ENABLE flag;
#else
    MAIL_BUTTON_ENABLE flag;
    uint16_t           id;
    std::string        sender_name;
    uint8_t            month;
    uint8_t            day;
    std::string        title;
    std::string        contents;
#endif

public:
#ifndef BOT
    bulletin_mail(const Mail& mail, MAIL_BUTTON_ENABLE flag);
#else
    bulletin_mail() = default;
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