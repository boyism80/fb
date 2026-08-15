#include <fb/game/protocol/unknown_58.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
unknown_58<V>::unknown_58(uint8_t flag, const std::string& text) :
    flag(flag),
    text(text)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void unknown_58<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->flag = reader.read<uint8_t>();
    this->text = reader.read<std::string, uint16_t>();
}
#else
template <CLIENT_VERSION V>
void unknown_58<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->flag);
    writer.write<std::string, uint16_t>(this->text);
}
#endif

template class unknown_58<CLIENT_VERSION::v550>;
template class unknown_58<CLIENT_VERSION::v565>;
template class unknown_58<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
