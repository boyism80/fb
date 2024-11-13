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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->name).write<uint32_t>(static_cast<uint32_t>(this->time.count() / 1000));
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->buff.model.name).write<uint32_t>(0x00);
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        auto spell = this->me.spells.at(index);
        if (spell == nullptr)
            return;

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1);
        writer.write<uint8_t>(static_cast<uint8_t>(spell->type));
        writer.write(spell->name);

        if (static_cast<int>(spell->type) < 3)
            writer.write<std::string>(spell->message);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->index = reader.read<uint8_t>();
        this->type  = reader.read<uint8_t>();
        this->name  = reader.read<std::string, uint8_t>();
        if (type < 3)
            this->message = reader.read<std::string, uint8_t>();
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        auto spell = this->me.spells.at(index);
        if (spell != nullptr)
            return;

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1).write<uint8_t>(0x00);
    }
};

}}}}} // namespace fb::protocol::game::response::spell

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__