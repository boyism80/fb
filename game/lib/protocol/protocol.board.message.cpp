#include <fb/game/protocol/board/board_message.h>

namespace fb::protocol::game::response {

board_message::board_message(const std::string& text, bool success, bool mail) :
    text(text),
    success(success),
    mail(mail)
{ }

async::task<void> board_message::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->mail ? 0x07 : 0x06);
    writer.write<uint8_t>(this->success);
    writer.write<std::string>(this->text);
    writer.write<uint8_t>(0x00);
}
} // namespace fb::protocol::game::response
