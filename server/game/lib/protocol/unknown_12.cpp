#include <fb/game/protocol/unknown_12.h>

namespace fb::protocol::game::response {

#ifndef BOT
async::task<void> unknown_12::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->party_slot);
    writer.write<uint8_t>(this->level_encoded);
}
#else
async::task<void> unknown_12::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->oid           = reader.read<uint32_t>();
    this->party_slot    = reader.read<uint8_t>();
    this->level_encoded = reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
