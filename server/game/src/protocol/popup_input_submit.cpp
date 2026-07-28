#include <fb/game/protocol/popup_input_submit.h>

namespace fb::protocol::game::request {

#ifdef BOT
void popup_input_submit::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->param0);
    writer.write<std::string, uint16_t>(this->text);
}
#else
void popup_input_submit::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->param0 = reader.read<uint8_t>();
    this->text   = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::request
