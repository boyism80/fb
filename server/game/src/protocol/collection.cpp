#include <fb/game/protocol/collection.h>

namespace fb::protocol::game::request {

#ifdef BOT
template <CLIENT_VERSION V>
collection<V>::collection(uint8_t slot) :
    slot(slot)
{ }

collection<CLIENT_VERSION::v651>::collection(uint8_t action, uint8_t slot) :
    action(action),
    slot(slot)
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void collection<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x01);
    writer.write<uint8_t>(this->slot);
}

void collection<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(this->action);
    if (this->action == 1)
        writer.write<uint8_t>(this->slot);
}
#else
template <CLIENT_VERSION V>
void collection<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // constant 0x00
    reader.read<uint8_t>(); // constant 0x01
    this->slot = reader.read<uint8_t>();
}

void collection<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // constant 0x00
    this->action = 0;
    this->slot   = 0;
    if (reader.readable_size() == 0)
        return;

    this->action = reader.read<uint8_t>();
    if (this->action == 1 && reader.readable_size() > 0)
        this->slot = reader.read<uint8_t>();
}
#endif

template class collection<CLIENT_VERSION::v550>;
template class collection<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::request
