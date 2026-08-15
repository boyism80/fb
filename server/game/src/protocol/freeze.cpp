#include <fb/game/protocol/freeze.h>

namespace fb::protocol::game::response {

#ifndef BOT
freeze::freeze(bool enable) :
    enable(enable)
{ }
#endif

#ifndef BOT
void freeze::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<bool>(this->enable);
}
#else
void freeze::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->enable = reader.read<bool>();
}
#endif

} // namespace fb::protocol::game::response
