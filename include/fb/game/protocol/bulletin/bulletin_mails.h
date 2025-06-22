#ifndef __PROTOCOL_GAME_MAILS_H__
#define __PROTOCOL_GAME_MAILS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>

using namespace fb::model::enum_value;
using namespace fb::protocol::internal;

namespace fb::protocol::game::response {

class bulletin_mails : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifndef BOT
    const MAIL_BUTTON_ENABLE        button_flags;
    const std::vector<MailSummary>& mails;
#else
    struct mail_data
    {
        bool        unread;
        uint16_t    id;
        std::string sender_name;
        uint8_t     month;
        uint8_t     day;
        std::string title;
    };

    MAIL_BUTTON_ENABLE     button_flags;
    std::string            mail_name;
    std::vector<mail_data> mails;
#endif

public:
#ifndef BOT
    bulletin_mails(const std::vector<MailSummary>& mails, MAIL_BUTTON_ENABLE button_flags);
#else
    bulletin_mails() = default;
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