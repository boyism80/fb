#ifndef __PROTOCOL_RESPONSE_GAME_OBJECT_H__
#define __PROTOCOL_RESPONSE_GAME_OBJECT_H__

#include <fb/protocol/protocol.h>
#include <fb/game/object.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace response { namespace object {

class direction : public fb::protocol::base::header
{
public:
#ifndef BOT
    const uint32_t              sequence;
    const fb::game::direction   value;
#else
    uint32_t                    sequence;
    fb::game::direction         value;
#endif

public:
#ifndef BOT
    direction(const fb::game::object& object) : direction(object.sequence(), object.direction())
    { }
    direction(uint32_t sequence, fb::game::direction value) : fb::protocol::base::header(0x11),
        sequence(sequence), value(value)
    { }
#else
    direction() : fb::protocol::base::header(0x11)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->sequence)
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->sequence  = in_stream.read_u32();
        this->value     = (fb::game::direction)in_stream.read_u8();
    }
#endif
};

class show : public fb::protocol::base::header
{
private:
    const fb::game::object*                 object;
    const std::vector<fb::game::object*>*   objects;

public:
    show(const fb::game::object& object) : fb::protocol::base::header(0x07),
        object(&object), objects(nullptr)
    { }

    show(const std::vector<fb::game::object*>& objects) : fb::protocol::base::header(0x07),
        object(nullptr), objects(&objects)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        if(this->object != nullptr)
        {
            auto map = this->object->map();
            if(map == nullptr)
                return;

            out_stream.write_u16(0x0001) // count
                      .write_u16(this->object->x()) // object x
                      .write_u16(this->object->y()) // object y
                      .write_u32(this->object->sequence()) // object sequence
                      .write_u16(this->object->look()) // npc icon code
                      .write_u8(this->object->color())  // color
                      .write_u8(this->object->direction()); // side
        }
        else if(this->objects != nullptr)
        {
            out_stream.write_u8(0x07)
                      .write_u16((uint16_t)this->objects->size());

            for(const auto object : *this->objects)
            {
                auto map = object->map();
                if(map == nullptr)
                    continue;

                out_stream.write_u16(object->x()) // object x
                          .write_u16(object->y()) // object y
                          .write_u32(object->sequence()) // object sequence
                          .write_u16(object->look()) // npc icon code
                          .write_u8(object->color())  // color
                          .write_u8(object->direction()); // side
            }
        }
        else
        {
            // error
            return;
        }
    }
};

class hide : public fb::protocol::base::header
{
public:
    const uint32_t              id;

public:
    hide(const fb::game::object& object) : hide(object.sequence())
    { }
    hide(uint32_t id) : fb::protocol::base::header(0x0E),
        id(id)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->id)
                  .write_u8(0x00);
    }
};

class chat : public fb::protocol::base::header
{
public:
    const fb::game::object&     me;
    const fb::game::chat::type  type;
    const std::string           message;

public:
    chat(const fb::game::object& me, const fb::game::chat::type type, const std::string message) : fb::protocol::base::header(0x0D),
        me(me), type(type), message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write_u32(this->me.sequence())
                  .write(this->message);
    }
};

class move : public fb::protocol::base::header
{
public:
#ifndef BOT
    const uint32_t              id;
    const point16_t             position;
    const fb::game::direction   direction;
#else
    uint32_t                    id;
    point16_t                   position;
    fb::game::direction         direction;
#endif

public:
#ifndef BOT
    move(const fb::game::object& object, const point16_t& position) : move(object.sequence(), object.direction(), position)
    { }
    move(const uint32_t id, fb::game::direction direction, const point16_t& position) : fb::protocol::base::header(0x0C),
        id(id), direction(direction), position(position)
    { }
#else
    move() : fb::protocol::base::header(0x0C)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->id)
                  .write_u16(this->position.x)
                  .write_u16(this->position.y)
                  .write_u8(this->direction)
                  .write_u8(0x00);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->id         = in_stream.read_u32();
        this->position.x = in_stream.read_u16();
        this->position.y = in_stream.read_u16();
        this->direction  = (fb::game::direction)in_stream.read_u8();
    }
#endif
};

class sound : public fb::protocol::base::header
{
public:
    const fb::game::object&         me;
    const fb::game::sound::type     value;

public:
    sound(const fb::game::object& me, fb::game::sound::type value) : fb::protocol::base::header(0x19),
        me(me), value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x00)
                  .write_u8(0x03)
                  .write_u16(this->value) // sound
                  .write_u8(100)
                  .write_u16(0x0004)
                  .write_u32(this->me.sequence())
                  .write_u16(0x0100)
                  .write_u16(0x0202)
                  .write_u16(0x0004)
                  .write_u16(0xCCCC);
    }
};

class effect : public fb::protocol::base::header
{
public:
    const fb::game::object&         me;
    const uint8_t                   value;

public:
    effect(const fb::game::object& me, uint8_t value) : fb::protocol::base::header(0x29),
        me(me), value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->me.sequence())
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_OBJECT_H__