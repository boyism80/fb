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
    entries(),
    group_id(0),
    bitmask(),
    id(static_cast<uint8_t>(oid)),
    slot(slot),
    onoff(flag)
{ }

unknown_12<CLIENT_VERSION::v651>::unknown_12(uint8_t type, uint8_t id, uint8_t slot, uint8_t onoff) :
    type(type),
    entries(),
    group_id(type == 1 ? id : 0),
    bitmask(),
    id(id),
    slot(slot),
    onoff(onoff)
{ }

unknown_12<CLIENT_VERSION::v651>::unknown_12(std::vector<unknown_12_entry> entries) :
    type(0),
    entries(std::move(entries)),
    group_id(0),
    bitmask(),
    id(0),
    slot(0),
    onoff(0)
{ }

unknown_12<CLIENT_VERSION::v651>::unknown_12(uint8_t group_id, std::vector<uint8_t> bitmask) :
    type(1),
    entries(),
    group_id(group_id),
    bitmask(std::move(bitmask)),
    id(0),
    slot(0),
    onoff(0)
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
    if (this->type == 0)
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->entries.size()));
        for (const auto& entry : this->entries)
        {
            writer.write<uint8_t>(entry.id);
            writer.write<uint8_t>(entry.extra);
        }
    }
    else if (this->type == 1)
    {
        writer.write<uint8_t>(this->group_id);
        writer.write<uint8_t>(static_cast<uint8_t>(this->bitmask.size()));
        for (auto byte : this->bitmask)
            writer.write<uint8_t>(byte);
    }
    else
    {
        writer.write<uint8_t>(this->id);
        writer.write<uint8_t>(this->slot);
        writer.write<uint8_t>(this->onoff);
    }
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
    this->type = reader.read<uint8_t>();
    this->entries.clear();
    this->bitmask.clear();
    this->group_id = 0;
    this->id       = 0;
    this->slot     = 0;
    this->onoff    = 0;
    if (this->type == 0)
    {
        auto count = reader.read<uint8_t>();
        for (uint8_t i = 0; i < count; i++)
        {
            auto entry  = unknown_12_entry();
            entry.id    = reader.read<uint8_t>();
            entry.extra = reader.read<uint8_t>();
            this->entries.push_back(entry);
        }
    }
    else if (this->type == 1)
    {
        this->group_id = reader.read<uint8_t>();
        auto count     = reader.read<uint8_t>();
        for (uint8_t i = 0; i < count; i++)
            this->bitmask.push_back(reader.read<uint8_t>());
    }
    else
    {
        this->id    = reader.read<uint8_t>();
        this->slot  = reader.read<uint8_t>();
        this->onoff = reader.read<uint8_t>();
    }
}
#endif

template class unknown_12<CLIENT_VERSION::v550>;
template class unknown_12<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
