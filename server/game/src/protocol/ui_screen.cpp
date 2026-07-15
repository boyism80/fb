#include <fb/game/protocol/ui_screen.h>

namespace fb::protocol::game::response {

#ifndef BOT
void ui_screen::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->screen));
}
#else
void ui_screen::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->screen = static_cast<UI_SCREEN>(reader.read<uint8_t>());
}
#endif

} // namespace fb::protocol::game::response
