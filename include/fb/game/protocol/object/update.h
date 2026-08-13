#ifndef __PROTOCOL_GAME_OBJECT_SHOW_H__
#define __PROTOCOL_GAME_OBJECT_SHOW_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class update : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x07;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const std::vector<fb::game::object*> objects;
#else
    struct object_data
    {
        uint16_t x;
        uint16_t y;
        uint8_t  direction;
        uint32_t oid;
        uint16_t look;
        uint8_t  color;
        uint8_t  facing;
    };

    uint16_t                 object_count;
    std::vector<object_data> objects_data;
#endif

public:
#ifndef BOT
    update(const fb::game::object& object);
    update(const std::vector<fb::game::object*>& objects);
#else
    update() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <CLIENT_VERSION V>
update<V>::update(const fb::game::object& object) :
    objects({const_cast<fb::game::object*>(&object)})
{ }

template <CLIENT_VERSION V>
update<V>::update(const std::vector<fb::game::object*>& objects) :
    objects(objects)
{ }

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
void update<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#endif

using update_v550 = update<CLIENT_VERSION::v550>;
using update_v565 = update<CLIENT_VERSION::v565>;
using update_v651 = update<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
