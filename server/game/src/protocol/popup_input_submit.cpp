#include <fb/game/protocol/popup_input_submit.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
void popup_input_submit<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->param0);
    writer.write<std::string, uint16_t>(this->text);
}
#else
template <CLIENT_VERSION V>
void popup_input_submit<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->param0 = reader.read<uint8_t>();
    this->text   = reader.read<std::string, uint16_t>();
}
#endif

template class popup_input_submit<CLIENT_VERSION::v550>;
template class popup_input_submit<CLIENT_VERSION::v565>;
template class popup_input_submit<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request
