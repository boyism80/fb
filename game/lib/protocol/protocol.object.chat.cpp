#include <fb/game/protocol/object/chat.h>

namespace fb::protocol::game::request {

#ifdef BOT
chat::chat(bool shout, const std::string& message) :
    shout(shout),
    message(message)
{ }
#endif

#ifdef BOT
async::task<void> chat::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->shout);
    writer.write<std::string, uint8_t>(this->message);
}
#else
async::task<void> chat::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->shout   = reader.read<uint8_t>();
    this->message = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
chat::chat(const fb::game::object& me, const std::string& text, CHAT_TYPE type) :
    me(me),
    text(text),
    type(type)
{ }
#endif

#ifndef BOT
async::task<void> chat::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint32_t>(this->me.sequence());
    writer.write<std::string>(this->text);
}
#else
async::task<void> chat::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->type     = (CHAT_TYPE)reader.read<uint8_t>();
    this->sequence = reader.read<uint32_t>();
    this->text     = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
