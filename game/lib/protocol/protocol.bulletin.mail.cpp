#include <fb/game/protocol/bulletin/bulletin_mail.h>

namespace fb::protocol::game::response {

bulletin_mail::bulletin_mail(const Mail& mail, MAIL_BUTTON_ENABLE flag) :
    mail(mail),
    flag(flag)
{ }

async::task<void> bulletin_mail::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    auto dt = fb::model::datetime(mail.created_date);

    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x05);
    writer.write<uint8_t>(static_cast<uint8_t>(flag));
    writer.write<uint8_t>(0x00);
    writer.write<uint16_t>(mail.id);
    writer.write<std::string>(mail.sender_name);
    writer.write<uint8_t>(static_cast<uint8_t>(dt.month()));
    writer.write<uint8_t>(static_cast<uint8_t>(dt.day()));
    writer.write<std::string>(mail.title);
    writer.write<std::string, uint16_t>(mail.contents);
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response