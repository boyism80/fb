#include <fb/game/protocol/c2s_relay.h>

namespace fb::protocol::game::response {

#ifndef BOT
c2s_relay::c2s_relay(std::string_view payload) :
    payload(std::string(payload))
{ }
#endif

#ifndef BOT
void c2s_relay::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<std::string, uint16_t>(this->payload);
}
#else
void c2s_relay::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->payload = reader.read<std::string, uint16_t>();
}
#endif

} // namespace fb::protocol::game::response
