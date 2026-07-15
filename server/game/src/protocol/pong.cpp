#include <fb/game/protocol/pong.h>

namespace fb::protocol::game::request {

#ifndef BOT
void pong::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->token   = reader.read<uint32_t>();
    this->unknown = reader.read<uint32_t>();
}
#else
void pong::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->token);
    writer.write<uint32_t>(this->unknown);
}
#endif

} // namespace fb::protocol::game::request
