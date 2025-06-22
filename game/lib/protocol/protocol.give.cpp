#include <fb/game/protocol/give.h>

namespace fb::protocol::game::request {

#ifdef BOT
async::task<void> give_item::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(this->slot);
    writer.write<bool>(this->all);
}
#else
async::task<void> give_item::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->slot = reader.read<uint8_t>();
    this->all  = reader.read<bool>();
}
#endif

#ifdef BOT
async::task<void> give_money::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->money);
}
#else
async::task<void> give_money::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->money = reader.read<uint32_t>();
}
#endif
} // namespace fb::protocol::game::request
