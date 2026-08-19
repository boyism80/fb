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

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
collection<V>::collection(uint32_t oid, uint8_t slot, uint8_t flag) :
    oid(oid),
    slot(slot),
    flag(flag)
{ }

collection<CLIENT_VERSION::v651>::collection(uint32_t oid, uint8_t slot, uint8_t flag) :
    type(COLLECTION_TYPE::FLAG),
    entries(),
    group_id(static_cast<uint8_t>(oid)),
    bitmask(),
    slot(slot),
    onoff(flag != 0)
{ }

collection<CLIENT_VERSION::v651>::collection(uint8_t group_id, uint8_t slot, bool onoff) :
    type(COLLECTION_TYPE::FLAG),
    entries(),
    group_id(group_id),
    bitmask(),
    slot(slot),
    onoff(onoff)
{ }

collection<CLIENT_VERSION::v651>::collection(std::vector<collection_entry> entries) :
    type(COLLECTION_TYPE::LIST),
    entries(std::move(entries)),
    group_id(0),
    bitmask(),
    slot(0),
    onoff(false)
{ }

collection<CLIENT_VERSION::v651>::collection(uint8_t group_id, std::vector<uint8_t> bitmask) :
    type(COLLECTION_TYPE::DIALOG),
    entries(),
    group_id(group_id),
    bitmask(std::move(bitmask)),
    slot(0),
    onoff(false)
{ }

template <CLIENT_VERSION V>
void collection<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint32_t>(this->oid);
    writer.write<uint8_t>(this->slot);
    writer.write<uint8_t>(this->flag);
}

void collection<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(0);
    writer.write<uint8_t>(static_cast<uint8_t>(this->type));
    if (this->type == COLLECTION_TYPE::LIST)
    {
        writer.write<uint8_t>(static_cast<uint8_t>(this->entries.size()));
        for (const auto& entry : this->entries)
        {
            writer.write<uint8_t>(entry.group_id);
            writer.write<uint8_t>(entry.unlocked_count);
        }
    }
    else if (this->type == COLLECTION_TYPE::DIALOG)
    {
        writer.write<uint8_t>(this->group_id);
        writer.write<uint8_t>(static_cast<uint8_t>(this->bitmask.size()));
        for (auto byte : this->bitmask)
            writer.write<uint8_t>(byte);
    }
    else
    {
        writer.write<uint8_t>(this->group_id);
        writer.write<uint8_t>(this->slot);
        writer.write<bool>(this->onoff);
    }
}
#else
template <CLIENT_VERSION V>
void collection<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->oid  = reader.read<uint32_t>();
    this->slot = reader.read<uint8_t>();
    this->flag = reader.read<uint8_t>();
}

void collection<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // gate 0
    this->type = static_cast<COLLECTION_TYPE>(reader.read<uint8_t>());
    this->entries.clear();
    this->bitmask.clear();
    this->group_id = 0;
    this->slot     = 0;
    this->onoff    = false;
    if (this->type == COLLECTION_TYPE::LIST)
    {
        auto count = reader.read<uint8_t>();
        for (uint8_t i = 0; i < count; i++)
        {
            auto entry           = collection_entry();
            entry.group_id       = reader.read<uint8_t>();
            entry.unlocked_count = reader.read<uint8_t>();
            this->entries.push_back(entry);
        }
    }
    else if (this->type == COLLECTION_TYPE::DIALOG)
    {
        this->group_id = reader.read<uint8_t>();
        auto count     = reader.read<uint8_t>();
        for (uint8_t i = 0; i < count; i++)
            this->bitmask.push_back(reader.read<uint8_t>());
    }
    else
    {
        this->group_id = reader.read<uint8_t>();
        this->slot     = reader.read<uint8_t>();
        this->onoff    = reader.read<bool>();
    }
}
#endif

template class collection<CLIENT_VERSION::v550>;
template class collection<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
