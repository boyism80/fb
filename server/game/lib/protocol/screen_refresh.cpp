#include <fb/game/protocol/screen_refresh.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> screen_refresh::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
}
#else
async::task<void> screen_refresh::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#endif
} // namespace fb::protocol::game::request
