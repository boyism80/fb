#include <fb/game/protocol/object/chat.h>

namespace fb::protocol::game::request {

#ifndef BOT
template <CLIENT_VERSION V>
void chat<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->shout   = reader.read<bool>();
    this->message = reader.read<std::string, uint8_t>();
}
#else
template <CLIENT_VERSION V>
chat<V>::chat(bool shout, std::string_view message) :
    shout(shout),
    message(std::string(message))
{ }

template <CLIENT_VERSION V>
void chat<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<bool>(this->shout);
    writer.write<std::string, uint8_t>(this->message);
}
#endif

template class chat<CLIENT_VERSION::v550>;
template class chat<CLIENT_VERSION::v565>;
template class chat<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

#ifndef BOT
chat::chat(const fb::game::object& me, std::string_view text, CHAT_TYPE type) :
    me(me),
    text(std::string(text)),
    type(type)
{ }
#endif

#ifndef BOT
void chat::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint32_t>(this->me.oid());
    writer.write<std::string>(this->text);
}
#else
void chat::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = (CHAT_TYPE)reader.read<uint8_t>();
    this->oid  = reader.read<uint32_t>();
    this->text = reader.read<std::string, uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
