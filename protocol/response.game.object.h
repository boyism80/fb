#ifndef __PROTOCOL_RESPONSE_GAME_OBJECT_H__
#define __PROTOCOL_RESPONSE_GAME_OBJECT_H__

#include "protocol/protocol.h"
#include "model/object/object.h"

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace response { namespace object {

class direction : public fb::protocol::base::header
{
public:
    const uint32_t              sequence;
    const fb::game::direction   value;

public:
    direction(const fb::game::object& object) : 
        direction(object.sequence(), object.direction())
    { }
    direction(uint32_t sequence, fb::game::direction value) : 
        sequence(sequence), value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x11)
                  .write_u32(this->sequence)
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
};

class show : public fb::protocol::base::header
{
private:
    const fb::game::object*                 object;
    const std::vector<fb::game::object*>*   objects;

public:
    show(const fb::game::object& object) : 
        object(&object), objects(nullptr)
    { }

    show(const std::vector<fb::game::object*>& objects) : 
        object(nullptr), objects(&objects)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        if(this->object != nullptr)
        {
            auto map = this->object->map();
            if(map == nullptr)
                return;

            out_stream.write_u8(0x07)
                      .write_u16(0x0001) // count
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
    hide(const fb::game::object& object) : 
        hide(object.sequence())
    { }
    hide(uint32_t id) : 
        id(id)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x0E)
                  .write_u32(this->id)
                  .write_u8(0x00);
    }
};

class chat : public fb::protocol::base::header
{
public:
    const fb::game::object&     me;
    const uint8_t               type;
    const std::string           message;

public:
    chat(const fb::game::object& me, const uint8_t type, const std::string message) : 
        me(me), type(type), message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x0D)
                  .write_u8(this->type)
                  .write_u32(this->me.sequence())
                  .write(this->message);
    }
};

class move : public fb::protocol::base::header
{
public:
    const uint32_t              id;
    const point16_t             before, current;
    const fb::game::direction   direction;
    const bool                  from_before;

public:
    move(const fb::game::object& object, bool from_before = true) : 
        move(object.sequence(), object.direction(), object.before().position, object.position(), from_before)
    { }
    move(const uint32_t id, fb::game::direction direction, const point16_t& before, const point16_t& current, bool from_before) : 
        id(id), direction(direction), before(before), current(current), from_before(from_before)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x0C)
                  .write_u32(this->id)
                  .write_u16(this->from_before ? this->before.x : this->current.x)
                  .write_u16(this->from_before ? this->before.y : this->current.y)
                  .write_u8(this->direction)
                  .write_u8(0x00);
    }
};

class sound : public fb::protocol::base::header
{
public:
    const fb::game::object&         me;
    const fb::game::sound::type     value;

public:
    sound(const fb::game::object& me, fb::game::sound::type value) : 
        me(me), value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x19)
                  .write_u8(0x00)
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
    effect(const fb::game::object& me, uint8_t value) : 
        me(me), value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x29)
                  .write_u32(this->me.sequence())
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_OBJECT_H__