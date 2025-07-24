#include <fb/game/protocol/message.h>

namespace fb::protocol::game::response {

#ifndef BOT
message::message(const std::string& text, MESSAGE_TYPE type) :
    text(text),
    type(type)
{ }
#endif

#ifndef BOT
async::task<void> message::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<std::string, uint16_t>(this->text);
}
#else
async::task<void> message::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->type = (MESSAGE_TYPE)reader.read<uint8_t>();
    this->text = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response