#include <fb/game/protocol/bulletin/article.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_article::bulletin_article(const fb::game::bulletin::article& value, BULLETIN_BUTTON_ENABLE flag) :
    value(value),
    flag(flag)
{ }
#endif

#ifndef BOT
void bulletin_article::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x03);
    writer.write<uint8_t>(static_cast<uint8_t>(flag));
    writer.write<uint8_t>(0x00);
    writer.write<uint16_t>(this->value.id);
    writer.write<std::string>(this->value.uname);
    writer.write<uint8_t>(this->value.month);
    writer.write<uint8_t>(this->value.day);
    writer.write<std::string>(this->value.title);
    writer.write<std::string, uint16_t>(this->value.contents);
    writer.write<uint8_t>(0x00);
}
#else
void bulletin_article::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // 0x03
    this->flag = static_cast<BULLETIN_BUTTON_ENABLE>(reader.read<uint8_t>());
    reader.read<uint8_t>(); // 0x00
    this->id       = reader.read<uint16_t>();
    this->uname    = reader.read<std::string, uint8_t>();
    this->month    = reader.read<uint8_t>();
    this->day      = reader.read<uint8_t>();
    this->title    = reader.read<std::string, uint8_t>();
    this->contents = reader.read<std::string, uint16_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response