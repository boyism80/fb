#ifndef __PROTOCOL_RESPONSE_GAME_LIFE_H__
#define __PROTOCOL_RESPONSE_GAME_LIFE_H__

#include <fb/protocol/protocol.h>
#include <fb/game/life.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace response { namespace life {

class action : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::life&       me;
    const fb::game::action      value;
    const fb::game::duration    duration;
    const uint8_t               sound;
#else
    uint32_t                    sequence;
    fb::game::action            value;
    fb::game::duration          duration;
    uint8_t                     sound;
#endif

public:
#ifndef BOT
    action(const fb::game::life& me, fb::game::action value, fb::game::duration duration, uint8_t sound = 0x00) : fb::protocol::base::header(0x1A),
        me(me), value(value), duration(duration), sound(sound)
    { }
#else
    action() : fb::protocol::base::header(0x1A)
    {}
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->me.sequence())
                  .write_u8(this->value) // type
                  .write_u16(this->duration) // duration
                  .write_u8(this->sound); // sound
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->sequence = in_stream.read_u32();
        this->value    = (fb::game::action)in_stream.read_u8();
        this->duration = (fb::game::duration)in_stream.read_u16();
        this->sound    = in_stream.read_u8();
    }
#endif
};

class show_hp : public fb::protocol::base::header
{
public:
    const fb::game::life&       me;
    const uint32_t              damage;
    const bool                  critical;
    const uint8_t               percentage;

public:
    show_hp(const fb::game::life& me, uint32_t damage, bool critical) : fb::protocol::base::header(0x13),
        me(me), damage(damage), critical(critical),
        percentage(this->me.hp() / float(this->me.base_hp()) * 100)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u32(this->me.sequence())
                  .write_u8(this->critical)
                  .write_u8(this->percentage)
                  .write_u32(this->damage)
                  .write_u8(0x00);
    }
};

class die : public fb::protocol::base::header
{
public:
    const uint32_t              id;

public:
    die(const fb::game::life& life) : die(life.sequence())
    { }
    die(uint32_t id) : fb::protocol::base::header(0x5F), 
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

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_LIFE_H__