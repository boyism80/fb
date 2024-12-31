#include <fb/game/protocol/board/board_mail.h>

namespace fb::protocol::game::response {

board_mail::board_mail(uint8_t button_flag) :
    button_flag(button_flag)
{ }

async::task<void> board_mail::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x05);
    writer.write<uint8_t>(button_flag);
    writer.write<uint8_t>(0x00);

    auto mail_id = 100;
    writer.write<uint16_t>(mail_id);

    writer.write<std::string>("name");
    writer.write<uint8_t>(12); // month
    writer.write<uint8_t>(31); // day
    writer.write<std::string>("title");
    writer.write<std::string, uint16_t>("mail contents");
    writer.write<uint8_t>(0x00);
}

} // namespace fb::protocol::game::response
