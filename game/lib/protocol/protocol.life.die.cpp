#include <fb/game/protocol/life/die.h>

namespace fb::protocol::game::response {

#ifndef BOT
die::die(const fb::game::life& life) :
    die(life.sequence())
{ }
die::die(uint32_t id) :
    id(id)
{ }
#endif

#ifndef BOT
async::task<void> die::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint32_t>(this->id);
    writer.write<uint8_t>(0x00);
}
#else
async::task<void> die::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->id = reader.read<uint32_t>();
    reader.read<uint8_t>(); // 0x00
}
#endif

} // namespace fb::protocol::game::response
