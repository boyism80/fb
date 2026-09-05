#include <fb/game/protocol/notice.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
notice<V>::notice(uint8_t flag, const std::string& text) :
    flag(flag),
    text(text)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void notice<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->flag = reader.read<uint8_t>();
    this->text = reader.read<std::string, uint16_t>();
}
#else
template <CLIENT_VERSION V>
void notice<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->flag);
    writer.write<std::string, uint16_t>(this->text);
}
#endif

template class notice<CLIENT_VERSION::v550>;
template class notice<CLIENT_VERSION::v565>;
template class notice<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
