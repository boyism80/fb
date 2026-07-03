#include <fb/game/protocol/bulletin/message.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_message::bulletin_message(std::string_view text, bool success, BULLETIN_MESSAGE_TYPE action) :
    text(std::string(text)),
    success(success),
    action(action)
{ }

async::task<void> bulletin_message::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->action));
    writer.write<uint8_t>(this->success);
    writer.write<std::string>(this->text);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> bulletin_message::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action  = reader.read<uint8_t>();
    this->success = reader.read<uint8_t>();
    this->text    = reader.read<std::string, uint8_t>();
    reader.read<uint8_t>();
}
#endif
} // namespace fb::protocol::game::response