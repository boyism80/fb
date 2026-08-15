#include <fb/game/protocol/unknown_12.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
unknown_12<V>::unknown_12(uint32_t oid, uint8_t slot, uint8_t flag) :
    oid(oid),
    slot(slot),
    flag(flag)
{ }

unknown_12<CLIENT_VERSION::v651>::unknown_12(uint32_t oid, uint8_t slot, uint8_t flag) :
    type(2),
    id(static_cast<uint8_t>(oid)),
    slot(slot),
    onoff(flag)
{ }

unknown_12<CLIENT_VERSION::v651>::unknown_12(uint8_t type, uint8_t id, uint8_t slot, uint8_t onoff) :
    type(type),
    id(id),
    slot(slot),
    onoff(onoff)
{ }

template <CLIENT_VERSION V>
void unknown_12<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->slot);
    writer.write<uint8_t>(this->flag);
}

void unknown_12<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(this->type);
    writer.write<uint8_t>(this->id);
    writer.write<uint8_t>(this->slot);
    writer.write<uint8_t>(this->onoff);
}
#else
template <CLIENT_VERSION V>
void unknown_12<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid  = reader.read<uint32_t>();
    this->slot = reader.read<uint8_t>();
    this->flag = reader.read<uint8_t>();
}

void unknown_12<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // gate 0
    this->type  = reader.read<uint8_t>();
    this->id    = reader.read<uint8_t>();
    this->slot  = reader.read<uint8_t>();
    this->onoff = reader.read<uint8_t>();
}
#endif

template class unknown_12<CLIENT_VERSION::v550>;
template class unknown_12<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
