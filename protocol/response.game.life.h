#ifndef __PROTOCOL_RESPONSE_GAME_LIFE_H__
#define __PROTOCOL_RESPONSE_GAME_LIFE_H__

#include <protocol/protocol.h>
#include <model/life/life.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace response { namespace game { namespace life {

 class action : public fb::protocol::base::response
 {
 public:
     const uint32_t              id;
     const fb::game::action      value;
     const fb::game::duration    duration;
     const uint8_t               sound;

 public:
     action(const fb::game::life& life, fb::game::action value, fb::game::duration duration, uint8_t sound = 0x00) : 
         action(life.id(), value, duration, sound)
     {}
     action(uint32_t id, fb::game::action value, fb::game::duration duration, uint8_t sound = 0x00) : 
         id(id), value(value), duration(duration), sound(sound)
     {}

 public:
     void serialize(fb::ostream& out_stream)
     {
         out_stream.write_u8(0x1A)
                   .write_u32(this->id)
                   .write_u8(this->value) // type
                   .write_u16(this->duration) // duration
                   .write_u8(this->sound); // sound
     }
 };

class show_hp : public fb::protocol::base::response
{
public:
    const uint32_t              id;
    const uint32_t              damage;
    const bool                  critical;
    const uint8_t               percentage;

public:
    show_hp(const fb::game::life& life, uint32_t damage, bool critical) : 
        show_hp(life.id(), damage, uint8_t((life.hp() / float(life.base_hp())) * 100), critical)
    {}
    show_hp(uint32_t id, uint32_t damage, uint8_t percentage, bool critical) : 
        id(id), damage(damage), percentage(percentage), critical(critical)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x13)
                  .write_u32(this->id)
                  .write_u8(this->critical)
                  .write_u8(this->percentage)
                  .write_u32(this->damage)
                  .write_u8(0x00);
    }
};

class die : public fb::protocol::base::response
{
public:
    const uint32_t              id;

public:
    die(const fb::game::life& life) : 
        die(life.id())
    {}
    die(uint32_t id) : id(id)
    {}

public:
    void serialize(fb::ostream& out_stream)
    {
        out_stream.write_u8(0x5F)
                  .write_u32(this->id)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_LIFE_H__