#include <fb/game/protocol/object/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
update<V>::update(const fb::game::object& object) :
    objects({const_cast<fb::game::object*>(&object)})
{ }

template <CLIENT_VERSION V>
update<V>::update(const std::vector<fb::game::object*>& objects) :
    objects(objects)
{ }
#endif

#ifndef BOT
template <CLIENT_VERSION V>
void update<V>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>((uint16_t)this->objects.size());

    for (const auto object : this->objects)
    {
        auto map = object->map();
        if (map == nullptr)
            continue;

        writer.write<uint16_t>(object->x());
        writer.write<uint16_t>(object->y());
        writer.write<uint32_t>(object->oid());
        writer.write<uint16_t>(object->look());
        writer.write<uint8_t>(object->color());
        writer.write<uint8_t>(static_cast<uint8_t>(object->direction()));
    }
    writer.write<uint8_t>(0);
}

template <>
void update<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>((uint16_t)this->objects.size());

    for (const auto object : this->objects)
    {
        auto map = object->map();
        if (map == nullptr)
            continue;

        writer.write<uint16_t>(object->x());
        writer.write<uint16_t>(object->y());
        switch (object->what())
        {
        case OBJECT_TYPE::NPC:
            writer.write<uint8_t>(static_cast<uint8_t>(fb::game::OBJECT_CURSOR::INTERACT));
            break;
        case OBJECT_TYPE::MOB:
            writer.write<uint8_t>(static_cast<uint8_t>(fb::game::OBJECT_CURSOR::ATTACK));
            break;
        default:
            writer.write<uint8_t>(static_cast<uint8_t>(fb::game::OBJECT_CURSOR::INSPECT));
            break;
        }
        writer.write<uint32_t>(object->oid());
        writer.write<uint16_t>(object->look());
        writer.write<uint8_t>(object->color());
        writer.write<uint8_t>(static_cast<uint8_t>(object->direction()));
    }
    writer.write<uint8_t>(0);
}
#else
template <CLIENT_VERSION V>
void update<V>::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    this->object_count = reader.read<uint16_t>();
    this->objects_data.clear();
    this->objects_data.reserve(this->object_count);

    for (int i = 0; i < this->object_count; i++)
    {
        object_data obj;
        obj.x = reader.read<uint16_t>();
        obj.y = reader.read<uint16_t>();
        if constexpr (V == CLIENT_VERSION::v651)
        {
            obj.direction = reader.read<uint8_t>();
            obj.oid       = reader.read<uint32_t>();
            obj.look      = reader.read<uint16_t>();
            obj.color     = reader.read<uint8_t>();
            obj.facing    = reader.read<uint8_t>();
        }
        else
        {
            obj.oid       = reader.read<uint32_t>();
            obj.look      = reader.read<uint16_t>();
            obj.color     = reader.read<uint8_t>();
            obj.direction = reader.read<uint8_t>();
            obj.facing    = obj.direction;
        }
        this->objects_data.push_back(obj);
    }
    reader.read<uint8_t>();
}
#endif

template class update<CLIENT_VERSION::v550>;
template class update<CLIENT_VERSION::v565>;
template class update<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response
