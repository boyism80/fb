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
        writer.write<uint32_t>(this->object->sequence());                       // object sequence
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
            writer.write<uint32_t>(object->sequence());                       // object sequence
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
    // TODO: deserialize bytes
}
#endif

} // namespace fb::protocol::game::response
