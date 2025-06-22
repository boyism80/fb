#include <fb/game/protocol/item/item_inactive.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> item_inactive::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
}
#else
[[nodiscard]] async::task<void> item_inactive::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->parts = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
}
#endif
} // namespace fb::protocol::game::request
