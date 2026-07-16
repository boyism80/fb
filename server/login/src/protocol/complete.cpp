#include <fb/login/protocol/complete.h>

namespace fb::protocol::login::request {

#ifndef BOT
void complete::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->hair     = reader.read<uint8_t>();
    this->gender   = reader.read<uint8_t>();
    this->nation   = reader.read<uint8_t>();
    this->creature = reader.read<uint8_t>();
}
#else
complete::complete(uint8_t hair, uint8_t gender, uint8_t nation, uint8_t creature) :
    hair(hair),
    gender(gender),
    nation(nation),
    creature(creature)
{ }

void complete::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->hair);
    writer.write<uint8_t>(this->gender);
    writer.write<uint8_t>(this->nation);
    writer.write<uint8_t>(this->creature);
}
#endif

} // namespace fb::protocol::login::request
