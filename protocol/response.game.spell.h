#ifndef __PROTOCOL_RESPONSE_GAME_SPELL_H__
#define __PROTOCOL_RESPONSE_GAME_SPELL_H__

#include "protocol/protocol.h"
#include "model/spell/spell.h"

namespace fb { namespace protocol { namespace game { namespace response { namespace spell {

class buff : public fb::protocol::base::header
{
public:
    const std::string       name;
    const uint32_t          time;

public:
    buff(const std::string& name, uint32_t time) : 
        name(name), time(time)
    { }
    buff(const fb::game::buff& buff) : 
        name(buff.spell().name()), time(buff.time())
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x3A)
                  .write(this->name)
                  .write_u32(this->time);
    }
};

class unbuff : public fb::protocol::base::header
{
public:
    const fb::game::buff&       buff;

public:
    unbuff(const fb::game::buff& buff) : 
        buff(buff)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x3A)
                  .write(this->buff.spell().name())
                  .write_u32(0x00);
    }
};

class update : public fb::protocol::base::header
{
public:
    const fb::game::life&           me;
    const uint8_t                   index;

public:
    update(const fb::game::life& me, uint8_t index) : 
        me(me), index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    spell = this->me.spells.at(index);
        if(spell == nullptr)
            return;

        out_stream.write_u8(0x17)
            .write_u8(this->index + 1)
            .write_u8(spell->type())
            .write(spell->name());

        if(static_cast<int>(spell->type()) < 3)
            out_stream.write(spell->message());
    }
};

class remove : public fb::protocol::base::header
{
public:
    const fb::game::life&           me;
    const uint8_t                   index;

public:
    remove(const fb::game::life& me, uint8_t index) : 
        me(me), index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    spell = this->me.spells.at(index);
        if(spell != nullptr)
            return;

        out_stream.write_u8(0x18)
                  .write_u8(this->index + 1)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__