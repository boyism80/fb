#ifndef __PROTOCOL_RESPONSE_GAME_SPELL_H__
#define __PROTOCOL_RESPONSE_GAME_SPELL_H__

#include <chrono>
#include <fb/protocol/protocol.h>
#include <fb/game/spell.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace spell {

class buff : public fb::protocol::base::header
{
public:
    const std::string               name;
    const std::chrono::milliseconds time;

public:
    buff(const std::string& name, uint32_t time) : fb::protocol::base::header(0x3A),
        name(name), time(time)
    { }
    buff(const fb::game::buff& buff) : fb::protocol::base::header(0x3A),
        name(buff.spell().name()), time(buff.time())
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write(this->name)
                  .write_u32(this->time.count() / 1000);
    }
};

class unbuff : public fb::protocol::base::header
{
public:
    const fb::game::buff&       buff;

public:
    unbuff(const fb::game::buff& buff) : fb::protocol::base::header(0x3A),
        buff(buff)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write(this->buff.spell().name())
                  .write_u32(0x00);
    }
};

class update : public fb::protocol::base::header
{
public:
    const fb::game::life&           me;
    const uint8_t                   index;

public:
    update(const fb::game::life& me, uint8_t index) : fb::protocol::base::header(0x17),
        me(me), index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    spell = this->me.spells.at(index);
        if(spell == nullptr)
            return;

        base::header::serialize(out_stream);
        out_stream.write_u8(this->index + 1)
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
    remove(const fb::game::life& me, uint8_t index) : fb::protocol::base::header(0x18),
        me(me), index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    spell = this->me.spells.at(index);
        if(spell != nullptr)
            return;

        base::header::serialize(out_stream);
        out_stream.write_u8(this->index + 1)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__