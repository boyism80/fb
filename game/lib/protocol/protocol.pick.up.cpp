#include <fb/game/protocol/pick_up.h>

namespace fb::protocol::game::request {

#ifdef BOT
pick_up::pick_up(bool boost) :
    boost(boost)
{ }
#endif

#ifdef BOT
async::task<void> pick_up::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->boost);
}
#else
async::task<void> pick_up::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->boost = bool(reader.read<uint8_t>());
}
#endif

} // namespace fb::protocol::game::request
