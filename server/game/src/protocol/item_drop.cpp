#include <fb/game/protocol/item/item_drop.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> item_drop::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index);
    writer.write<bool>(this->all);
}
#else
async::task<void> item_drop::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->all   = bool(reader.read<uint8_t>());
}
#endif
} // namespace fb::protocol::game::request
