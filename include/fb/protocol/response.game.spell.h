#ifndef __PROTOCOL_RESPONSE_GAME_SPELL_H__
#define __PROTOCOL_RESPONSE_GAME_SPELL_H__

#include <chrono>
#include <fb/protocol/protocol.h>
#include <spell.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace spell {

class buff : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3A;

public:
    const std::string               name;
    const std::chrono::milliseconds time;

public:
    buff(const std::string& name, uint32_t time) :
        name(name),
        time(time)
    { }
    buff(const fb::game::buff& buff) :
        name(buff.model.name),
        time(buff.time())
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write(this->name).write_u32(static_cast<uint32_t>(this->time.count() / 1000));
    }
};

class unbuff : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3A;

public:
    const fb::game::buff& buff;

public:
    unbuff(const fb::game::buff& buff) :
        buff(buff)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write(this->buff.model.name).write_u32(0x00);
    }
};

class update : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x17;

public:
#ifdef BOT
    uint8_t     index;
    uint8_t     type;
    std::string name;
    std::string message;
#else
    const fb::game::life& me;
    const uint8_t         index;
#endif

public:
#ifdef BOT
    update() = default;
#else
    update(const fb::game::life& me, uint8_t index) :
        me(me),
        index(index)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        auto spell = this->me.spells.at(index);
        if (spell == nullptr)
            return;

        out_stream.write_u8(header);
        out_stream.write_u8(this->index + 1).write_u8(spell->type).write(spell->name);

        if (static_cast<int>(spell->type) < 3)
            out_stream.write(spell->message);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8();
        this->type  = in_stream.read_u8();
        this->name  = in_stream.readstr_u8();
        if (type < 3)
            this->message = in_stream.readstr_u8();
    }
#endif
};

class remove : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x18;

public:
    const fb::game::life& me;
    const uint8_t         index;

public:
    remove(const fb::game::life& me, uint8_t index) :
        me(me),
        index(index)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto spell = this->me.spells.at(index);
        if (spell != nullptr)
            return;

        out_stream.write_u8(header);
        out_stream.write_u8(this->index + 1).write_u8(0x00);
    }
};

}}}}} // namespace fb::protocol::game::response::spell

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__