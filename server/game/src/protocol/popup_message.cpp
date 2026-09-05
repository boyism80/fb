#include <fb/game/protocol/popup_message.h>

namespace fb::protocol::game::response {

#ifndef BOT
popup_message::popup_message(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text) :
    param0(param0),
    param1(param1),
    param2(param2),
    param3(param3),
    text(std::string(text))
{ }
#endif

#ifndef BOT
popup_input::popup_input(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text) :
    param0(param0),
    param1(param1),
    param2(param2),
    param3(param3),
    text(std::string(text))
{ }
#endif

#ifndef BOT
void popup_message::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->param0);
    writer.write<uint8_t>(this->param1);
    writer.write<uint8_t>(this->param2);
    writer.write<uint8_t>(this->param3);
    writer.write<std::string, uint16_t>(this->text);
}

void popup_input::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->param0);
    writer.write<uint8_t>(this->param1);
    writer.write<uint8_t>(this->param2);
    writer.write<uint8_t>(this->param3);
    writer.write<std::string, uint16_t>(this->text);
}
#else
void popup_message::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->param0 = reader.read<uint8_t>();
    this->param1 = reader.read<uint8_t>();
    this->param2 = reader.read<uint8_t>();
    this->param3 = reader.read<uint8_t>();
    this->text   = reader.read<std::string, uint16_t>();
}

void popup_input::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->param0 = reader.read<uint8_t>();
    this->param1 = reader.read<uint8_t>();
    this->param2 = reader.read<uint8_t>();
    this->param3 = reader.read<uint8_t>();
    this->text   = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
