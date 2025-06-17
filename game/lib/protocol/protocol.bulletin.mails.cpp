#include <fb/game/protocol/bulletin/bulletin_mails.h>

namespace fb::protocol::game::response {

bulletin_mails::bulletin_mails(const std::vector<MailSummary>& mails, MAIL_BUTTON_ENABLE button_flags) :
    mails(mails),
    button_flags(button_flags)
{ }

async::task<void> bulletin_mails::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(this->button_flags));
    writer.write<uint16_t>(0xFFFF);
    writer.write<std::string>("mail name");

    writer.write<uint8_t>(this->mails.size());
    for (auto& mail : this->mails)
    {
        writer.write<bool>(!mail.read);
        writer.write<uint16_t>(mail.id);
        writer.write<std::string>(mail.sender_name);

        auto dt = fb::model::datetime(mail.created_date);
        writer.write<uint8_t>(static_cast<uint8_t>(dt.month()));
        writer.write<uint8_t>(static_cast<uint8_t>(dt.day()));
        writer.write<std::string>(mail.title);
    }
    writer.write<uint8_t>(0x00);
}
} // namespace fb::protocol::game::response