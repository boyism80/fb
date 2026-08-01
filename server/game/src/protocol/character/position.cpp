#include <fb/game/protocol/character/position.h>

namespace fb::protocol::game::response {

#ifndef BOT
position::position(const fb::game::character& ch) :
    ch(ch)
{ }
#endif

#ifndef BOT
void position::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(this->ch.x());
    writer.write<uint16_t>(this->ch.y());

    auto viewport = this->ch.viewport();
    writer.write<uint16_t>(viewport.x);
    writer.write<uint16_t>(viewport.y);
}
#else
void position::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->abs.x = reader.read<uint16_t>();
    this->abs.y = reader.read<uint16_t>();
    this->rel.x = reader.read<uint16_t>();
    this->rel.y = reader.read<uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
