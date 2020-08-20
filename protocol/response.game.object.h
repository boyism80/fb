#ifndef __PROTOCOL_RESPONSE_GAME_OBJECT_H__
#define __PROTOCOL_RESPONSE_GAME_OBJECT_H__

#include <protocol/protocol.h>
#include <model/object/object.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace response { namespace game { namespace object {

class direction : public fb::protocol::base::response
{
public:
    const uint32_t              id;
    const fb::game::direction   value;

public:
    direction(const fb::game::object& object) : 
        direction(object.id(), object.direction())
    {}
    direction(uint32_t id, fb::game::direction value) : 
        id(id), value(value)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x11)
                  .write_u32(this->id)
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
};

class show : public fb::protocol::base::response
{
public:
    const uint32_t              id;
    const uint16_t              look;
    const uint8_t               color;
    const point16_t             position;
    const fb::game::direction   direction;

public:
    show(const fb::game::object& object) : 
        show(object.id(), object.look(), object.color(), object.position(), object.direction())
    {}
    show(uint32_t id, uint16_t look, uint8_t color, const point16_t position, fb::game::direction direction) : 
        id(id), look(look), color(color), position(position), direction(direction)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x0E)
                  .write_u32(this->id)
                  .write_u8(0x00);
    }
};

class hide : public fb::protocol::base::response
{
public:
    const uint32_t              id;

public:
    hide(const fb::game::object& object) : 
        hide(object.id())
    {}
    hide(uint32_t id) : 
        id(id)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x0E)
                  .write_u32(this->id)
                  .write_u8(0x00);
    }
};

class chat : public fb::protocol::base::response
{
public:
    const uint32_t              id;
    const uint8_t               type;
    const std::string           message;

public:
    chat(const fb::game::object& object, const uint8_t type, const std::string message) : 
        chat(object.id(), type, message)
    {}
    chat(const uint32_t id, const uint8_t type, const std::string message) : 
        id(id), type(type), message(message)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x0D)
                  .write_u8(this->type)
                  .write_u32(this->id)
                  .write(this->message);
    }
};

class move : public fb::protocol::base::response
{
public:
    const uint32_t              id;
    const point16_t             before, current;
    const fb::game::direction   direction;
    const bool                  from_before;

public:
    move(const fb::game::object& object, bool from_before) : 
        move(object.id(), object.direction(), object.before(), object.position(), from_before)
    {}
    move(const uint32_t id, fb::game::direction direction, const point16_t& before, const point16_t& current, bool from_before) : 
        id(id), direction(direction), before(before), current(current), from_before(from_before)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x0C)
                  .write_u32(this->id)
                  .write_u16(this->from_before ? this->before.x : this->current.x)
                  .write_u16(this->from_before ? this->before.y : this->current.y)
                  .write_u8(this->direction)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_OBJECT_H__