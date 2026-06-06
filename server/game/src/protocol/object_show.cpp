#include <fb/game/protocol/object/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
update::update(const fb::game::object& object) :
    objects({const_cast<fb::game::object*>(&object)})
{ }

update::update(const std::vector<fb::game::object*>& objects) :
    objects(objects)
{ }
#endif

#ifndef BOT
async::task<void> update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint16_t>((uint16_t)this->objects.size());

    for (const auto object : this->objects)
    {
        auto map = object->map();
        if (map == nullptr)
            continue;

        writer.write<uint16_t>(object->x());                              // object x
        writer.write<uint16_t>(object->y());                              // object y
        writer.write<uint32_t>(object->oid());                            // object oid
        writer.write<uint16_t>(object->look());                           // npc icon code
        writer.write<uint8_t>(object->color());                           // color
        writer.write<uint8_t>(static_cast<uint8_t>(object->direction())); // side
    }
    writer.write<uint8_t>(0);
}
#else
async::task<void> update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    // Read object count
    this->object_count = reader.read<uint16_t>();
    this->objects_data.clear();
    this->objects_data.reserve(this->object_count);

    // Read each object's data
    for (int i = 0; i < this->object_count; i++)
    {
        object_data obj;
        obj.x         = reader.read<uint16_t>();
        obj.y         = reader.read<uint16_t>();
        obj.oid       = reader.read<uint32_t>();
        obj.look      = reader.read<uint16_t>();
        obj.color     = reader.read<uint8_t>();
        obj.direction = reader.read<uint8_t>();
        this->objects_data.push_back(obj);
    }

    // Read trailing byte
    reader.read<uint8_t>();
}
#endif

} // namespace fb::protocol::game::response
