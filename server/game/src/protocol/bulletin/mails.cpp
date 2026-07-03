#include <fb/game/protocol/bulletin/mails.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_mails::bulletin_mails(const std::vector<MailSummary>& mails, MAIL_BUTTON_ENABLE button_flags) :
    mails(mails),
    button_flags(button_flags)
{ }

async::task<void> bulletin_mails::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(this->button_flags));
    writer.write<uint16_t>(0xFFFF);
    writer.write<std::string>("mail name");

    writer.write<uint8_t>(this->mails.size());
    for (auto& mail : this->mails)
    {
        writer.write<bool>(!mail.read);
        writer.write<uint16_t>(mail.id);
        writer.write<std::string>(mail.sender);

        auto dt = fb::model::datetime(mail.created_date);
        writer.write<uint8_t>(static_cast<uint8_t>(dt.month()));
        writer.write<uint8_t>(static_cast<uint8_t>(dt.day()));
        writer.write<std::string>(mail.title);
    }
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> bulletin_mails::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    reader.read<uint8_t>(); // 0x04
    this->button_flags = static_cast<MAIL_BUTTON_ENABLE>(reader.read<uint8_t>());
    reader.read<uint16_t>(); // 0xFFFF
    this->mail_name = reader.read<std::string, uint8_t>();

    uint8_t count = reader.read<uint8_t>();
    this->mails.clear();

    for (int i = 0; i < count; i++)
    {
        mail_data mail;
        mail.unread = reader.read<bool>();
        mail.id     = reader.read<uint16_t>();
        mail.sender = reader.read<std::string, uint8_t>();
        mail.month  = reader.read<uint8_t>();
        mail.day    = reader.read<uint8_t>();
        mail.title  = reader.read<std::string, uint8_t>();
        this->mails.push_back(mail);
    }

    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response