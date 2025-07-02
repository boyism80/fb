#include <fb/game/protocol/object/update.h>

namespace fb::protocol::game::response {

#ifndef BOT
update::update(const fb::game::object& object) :
    object(&object),
    objects(nullptr)
{ }

update::update(const std::vector<fb::game::object*>& objects) :
    object(nullptr),
    objects(&objects)
{ }
#endif

#ifndef BOT
async::task<void> update::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    if (this->object != nullptr)
    {
        auto map = this->object->map();
        if (map == nullptr)
            co_return;

        writer.write<uint16_t>(0x0001);                                         // count
        writer.write<uint16_t>(this->object->x());                              // object x
        writer.write<uint16_t>(this->object->y());                              // object y
        writer.write<uint32_t>(this->object->oid());                            // object oid
        writer.write<uint16_t>(this->object->look());                           // npc icon code
        writer.write<uint8_t>(this->object->color());                           // color
        writer.write<uint8_t>(static_cast<uint8_t>(this->object->direction())); // side
    }
    else if (this->objects != nullptr)
    {
        writer.write<uint8_t>(0x07);
        writer.write<uint16_t>((uint16_t)this->objects->size());

        for (const auto object : *this->objects)
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
    }
    else
    {
        // error
        co_return;
    }
}
#else
async::task<void> update::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);

    // 첫 번째 바이트로 타입 구분
    uint8_t type = reader.read<uint8_t>();
    if (type == 0x07)
    {
        // 다중 오브젝트
        this->object_count = reader.read<uint16_t>();
        this->objects_data.clear();

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
    }
    else
    {
        // 단일 오브젝트 (type은 count의 상위 바이트)
        uint16_t count = (type << 8) | reader.read<uint8_t>();
        if (count == 1)
        {
            this->object_count = 1;
            object_data obj;
            obj.x         = reader.read<uint16_t>();
            obj.y         = reader.read<uint16_t>();
            obj.oid       = reader.read<uint32_t>();
            obj.look      = reader.read<uint16_t>();
            obj.color     = reader.read<uint8_t>();
            obj.direction = reader.read<uint8_t>();
            this->objects_data.push_back(obj);
        }
    }
}
#endif

} // namespace fb::protocol::game::response
