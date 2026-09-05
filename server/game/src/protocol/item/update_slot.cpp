#include <fb/game/protocol/item/update_slot.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
item_update_slot<V>::item_update_slot(const fb::game::character& me, EQUIPMENT_PARTS parts) :
    me(me),
    parts(parts)
{ }

namespace {

std::shared_ptr<fb::game::item> resolve_equipment(const fb::game::character& me, EQUIPMENT_PARTS parts)
{
    switch (parts)
    {
    case EQUIPMENT_PARTS::WEAPON:
        return me.items.weapon();
    case EQUIPMENT_PARTS::ARMOR:
        return me.items.armor();
    case EQUIPMENT_PARTS::SHIELD:
        return me.items.shield();
    case EQUIPMENT_PARTS::HELMET:
        return me.items.helmet();
    case EQUIPMENT_PARTS::LEFT_HAND:
        return me.items.ring(EQUIPMENT_POSITION::LEFT);
    case EQUIPMENT_PARTS::RIGHT_HAND:
        return me.items.ring(EQUIPMENT_POSITION::RIGHT);
    case EQUIPMENT_PARTS::LEFT_AUX:
        return me.items.auxiliary(EQUIPMENT_POSITION::LEFT);
    case EQUIPMENT_PARTS::RIGHT_AUX:
        return me.items.auxiliary(EQUIPMENT_POSITION::RIGHT);
    default:
        return nullptr;
    }
}

} // namespace

template <CLIENT_VERSION V>
void item_update_slot<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto item = resolve_equipment(this->me, this->parts);
    if (item == nullptr)
        return;

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>(item->look());
    writer.write<uint8_t>(item->color());
    writer.write<std::string, uint8_t>(item->name());
}

template <>
void item_update_slot<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    auto item = resolve_equipment(this->me, this->parts);
    if (item == nullptr)
        return;

    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(static_cast<uint8_t>(this->parts));
    writer.write<uint16_t>(item->look());
    writer.write<uint8_t>(item->color());
    writer.write<std::string, uint8_t>(item->name());
    writer.write<std::string, uint8_t>(this->unknown_name_b);
    writer.write<uint32_t>(this->unknown_u32);
}

template item_update_slot<CLIENT_VERSION::v550>::item_update_slot(const fb::game::character&, EQUIPMENT_PARTS);
template item_update_slot<CLIENT_VERSION::v565>::item_update_slot(const fb::game::character&, EQUIPMENT_PARTS);
template item_update_slot<CLIENT_VERSION::v651>::item_update_slot(const fb::game::character&, EQUIPMENT_PARTS);
template void item_update_slot<CLIENT_VERSION::v550>::serialize(fb::stream_writer<big_endian>&) const;
template void item_update_slot<CLIENT_VERSION::v565>::serialize(fb::stream_writer<big_endian>&) const;
#else
template <CLIENT_VERSION V>
void item_update_slot<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    this->name  = reader.read<std::string, uint8_t>();
}

template <>
void item_update_slot<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->parts          = static_cast<EQUIPMENT_PARTS>(reader.read<uint8_t>());
    this->look           = reader.read<uint16_t>();
    this->color          = reader.read<uint8_t>();
    this->name           = reader.read<std::string, uint8_t>();
    this->unknown_name_b = reader.read<std::string, uint8_t>();
    this->unknown_u32    = reader.read<uint32_t>();
}

template void item_update_slot<CLIENT_VERSION::v550>::deserialize(fb::stream_reader<big_endian>&);
template void item_update_slot<CLIENT_VERSION::v565>::deserialize(fb::stream_reader<big_endian>&);
#endif

} // namespace fb::protocol::game::response
