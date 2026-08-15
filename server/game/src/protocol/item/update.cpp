#include <fb/game/protocol/item/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
item_update<V>::item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count, bool valid) :
    index(index),
    look(look),
    color(color),
    name(std::move(name)),
    count(count),
    valid(valid)
{ }

template <CLIENT_VERSION V>
item_update<V> item_update<V>::from_inventory(const fb::game::character& me, uint8_t index)
{
    auto item = me.items.at(index);
    if (item == nullptr)
        return item_update<V>(index, 0, 0, std::string{}, 0, false);

    return item_update<V>(index,
                          item->look(),
                          item->color(),
                          item->inven_name(),
                          static_cast<uint32_t>(item->count() - item->trade_count()),
                          true);
}

template <CLIENT_VERSION V>
item_update<V>::item_update(const fb::game::character& me, uint8_t index) :
    item_update(from_inventory(me, index))
{ }

template <CLIENT_VERSION V>
item_update<V>::item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count) :
    item_update(index, look, color, std::move(name), count, true)
{ }

template <CLIENT_VERSION V>
void item_update<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    if (this->valid == false)
        return;

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<uint32_t>(this->count);
    writer.write<uint8_t>(0x00);
    writer.write<uint8_t>(0x00);
}

item_update<CLIENT_VERSION::v651>::item_update(uint8_t     index,
                                               uint16_t    look,
                                               uint8_t     color,
                                               std::string name,
                                               uint32_t    count,
                                               bool        valid) :
    index(index),
    look(look),
    color(color),
    name(std::move(name)),
    name_b(),
    count(count),
    flag(0),
    extra(0),
    valid(valid)
{ }

item_update<CLIENT_VERSION::v651> item_update<CLIENT_VERSION::v651>::from_inventory(const fb::game::character& me,
                                                                                    uint8_t                    index)
{
    auto item = me.items.at(index);
    if (item == nullptr)
        return item_update<CLIENT_VERSION::v651>(index, 0, 0, std::string{}, 0, false);

    return item_update<CLIENT_VERSION::v651>(index,
                                             item->look(),
                                             item->color(),
                                             item->inven_name(),
                                             static_cast<uint32_t>(item->count() - item->trade_count()),
                                             true);
}

item_update<CLIENT_VERSION::v651>::item_update(const fb::game::character& me, uint8_t index) :
    item_update(from_inventory(me, index))
{ }

item_update<CLIENT_VERSION::v651>::item_update(uint8_t     index,
                                               uint16_t    look,
                                               uint8_t     color,
                                               std::string name,
                                               uint32_t    count) :
    item_update(index, look, color, std::move(name), count, true)
{ }

void item_update<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    if (this->valid == false)
        return;

    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(this->index + 1);
    writer.write<uint16_t>(this->look);
    writer.write<uint8_t>(this->color);
    writer.write<std::string, uint8_t>(this->name);
    writer.write<std::string, uint8_t>(this->name_b);
    writer.write<uint32_t>(this->count);
    writer.write<uint8_t>(this->flag);
    writer.write<uint32_t>(this->extra);
}
#else
template <CLIENT_VERSION V>
void item_update<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index = reader.read<uint8_t>() - 1;
    this->look  = reader.read<uint16_t>();
    this->color = reader.read<uint8_t>();
    this->name  = reader.read<std::string, uint8_t>();
    this->count = reader.read<uint32_t>();
    reader.read<uint8_t>();
    reader.read<uint8_t>();
}

void item_update<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->index  = reader.read<uint8_t>() - 1;
    this->look   = reader.read<uint16_t>();
    this->color  = reader.read<uint8_t>();
    this->name   = reader.read<std::string, uint8_t>();
    this->name_b = reader.read<std::string, uint8_t>();
    this->count  = reader.read<uint32_t>();
    this->flag   = reader.read<uint8_t>();
    this->extra  = reader.read<uint32_t>();
}
#endif

template class item_update<CLIENT_VERSION::v550>;
template class item_update<CLIENT_VERSION::v565>;

} // namespace fb::protocol::game::response
