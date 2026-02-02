#include <fb/game/protocol/bulletin/bulletin_message.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_message::bulletin_message(std::string_view text, bool success, bool unknown) :
    text(std::string(text)),
    success(success),
    unknown(unknown)
{ }

async::task<void> bulletin_message::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->unknown ? 0x07 : 0x06);
    writer.write<uint8_t>(this->success);
    writer.write<std::string>(this->text);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> bulletin_message::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    uint8_t type  = reader.read<uint8_t>();
    this->unknown = (type == 0x07);
    this->success = reader.read<uint8_t>();
    this->text    = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif
} // namespace fb::protocol::game::response