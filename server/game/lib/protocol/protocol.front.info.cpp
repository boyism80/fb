#include <fb/game/protocol/front_info.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> front_info::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
}
#else
async::task<void> front_info::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
}
#endif

} // namespace fb::protocol::game::request
