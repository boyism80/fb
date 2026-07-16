#include <fb/game/protocol/swap.h>

namespace fb::protocol::game::request {

#ifdef BOT
void swap::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    writer.write<uint8_t>(this->src);
    writer.write<uint8_t>(this->dst);
}
#else
void swap::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->type = SWAP_TYPE(reader.read<uint8_t>());
    this->src  = reader.read<uint8_t>();
    this->dst  = reader.read<uint8_t>();
}
#endif
} // namespace fb::protocol::game::request
