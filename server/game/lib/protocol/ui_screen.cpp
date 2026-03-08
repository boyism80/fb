#include <fb/game/protocol/ui_screen.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> ui_screen::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->screen));
}
#else
async::task<void> ui_screen::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->screen = static_cast<UI_SCREEN>(reader.read<uint8_t>());
}
#endif

} // namespace fb::protocol::game::response
