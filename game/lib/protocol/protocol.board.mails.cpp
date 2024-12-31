#include <fb/game/protocol/board/board_mails.h>

namespace fb::protocol::game::response {

board_mails::board_mails(MAIL_BUTTON_ENABLE button_flags) :
    button_flags(button_flags)
{ }

async::task<void> board_mails::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x04);
    writer.write<uint8_t>(static_cast<uint8_t>(this->button_flags));
    writer.write<uint16_t>(65535);
    writer.write<std::string>("mail name");

    auto count = 23;
    writer.write<uint8_t>(count);
    for (int i = 0; i < count; i++)
    {
        auto read = (i % 2) == 0;
        writer.write<bool>(read);
        writer.write<uint16_t>(i + 100); // id
        writer.write<std::string>(std::format("name {}", i));
        writer.write<uint8_t>(12); // month
        writer.write<uint8_t>(31); // day
        writer.write<std::string>(std::format("mail title {}", i));
    }
    writer.write<uint8_t>(0x00);
}
} // namespace fb::protocol::game::response
