#include <fb/game/protocol/group_portrait.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
group_portrait<V>::group_portrait(uint8_t subtype, uint8_t count) :
    subtype(subtype),
    count(count)
{ }

group_portrait<CLIENT_VERSION::v651>::group_portrait(uint8_t                           subtype,
                                                     uint8_t                           count,
                                                     std::vector<group_portrait_entry> entries) :
    subtype(subtype),
    count(count),
    entries(std::move(entries))
{ }

group_portrait<CLIENT_VERSION::v651>::group_portrait(std::string name, uint32_t cur_hp) :
    subtype(3),
    count(1),
    entries([&] {
        group_portrait_entry e;
        e.name   = std::move(name);
        e.cur_hp = cur_hp;
        return std::vector<group_portrait_entry>{std::move(e)};
    }())
{ }
#endif

#ifdef BOT
template <CLIENT_VERSION V>
void group_portrait<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->count   = reader.read<uint8_t>();
}

void group_portrait<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->subtype = reader.read<uint8_t>();
    this->entries.clear();
    if (this->subtype == 3)
    {
        group_portrait_entry e;
        e.name      = reader.read<std::string, uint8_t>();
        e.cur_hp    = reader.read<uint32_t>();
        this->count = 1;
        this->entries.push_back(std::move(e));
        return;
    }

    this->count = reader.read<uint8_t>();
    this->entries.reserve(this->count);
    for (uint8_t i = 0; i < this->count; ++i)
    {
        group_portrait_entry e;
        e.name            = reader.read<std::string, uint8_t>();
        e.leader          = reader.read<uint8_t>();
        e.hair            = reader.read<uint16_t>();
        e.color           = reader.read<uint8_t>();
        e.face_hair_tint  = reader.read<uint8_t>();
        e.hair_to_hat     = reader.read<uint8_t>();
        e.helmet          = reader.read<uint8_t>();
        e.helmet_color    = reader.read<uint8_t>();
        e.accessory_pack  = reader.read<uint16_t>();
        e.accessory_color = reader.read<uint8_t>();
        e.max_hp          = reader.read<uint32_t>();
        e.cur_hp          = reader.read<uint32_t>();
        this->entries.push_back(std::move(e));
    }
}
#else
template <CLIENT_VERSION V>
void group_portrait<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    writer.write<uint8_t>(this->count);
}

void group_portrait<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->subtype);
    if (this->subtype == 3)
    {
        auto name = this->entries.empty() ? std::string{} : this->entries[0].name;
        auto hp   = this->entries.empty() ? uint32_t{0} : this->entries[0].cur_hp;
        writer.write<std::string, uint8_t>(name);
        writer.write<uint32_t>(hp);
        return;
    }

    auto n = static_cast<uint8_t>(this->entries.size());
    writer.write<uint8_t>(n);
    for (const auto& e : this->entries)
    {
        writer.write<std::string, uint8_t>(e.name);
        writer.write<uint8_t>(e.leader);
        writer.write<uint16_t>(e.hair);
        writer.write<uint8_t>(e.color);
        writer.write<uint8_t>(e.face_hair_tint);
        writer.write<uint8_t>(e.hair_to_hat);
        writer.write<uint8_t>(e.helmet);
        writer.write<uint8_t>(e.helmet_color);
        writer.write<uint16_t>(e.accessory_pack);
        writer.write<uint8_t>(e.accessory_color);
        writer.write<uint32_t>(e.max_hp);
        writer.write<uint32_t>(e.cur_hp);
    }
}
#endif

template class group_portrait<CLIENT_VERSION::v550>;
template class group_portrait<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
