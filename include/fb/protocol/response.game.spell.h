#ifndef __PROTOCOL_RESPONSE_GAME_SPELL_H__
#define __PROTOCOL_RESPONSE_GAME_SPELL_H__

#include <chrono>
#include <fb/protocol/protocol.h>
#ifndef BOT
#include <spell.h>
#endif

namespace fb::protocol::game::response::spell {

class buff : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3A;

public:
#ifndef BOT
    const std::string               name;
    const std::chrono::milliseconds time;
#else

#endif

public:
#ifndef BOT
    buff(const std::string& name, uint32_t time) :
        name(name),
        time(time)
    { }
    buff(const fb::game::buff& buff) :
        name(buff.model.name),
        time(buff.time())
    { }
#else
    buff() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->name);
        writer.write<uint32_t>(static_cast<uint32_t>(this->time.count() / 1000));
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class unbuff : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3A;

public:
#ifndef BOT
    const fb::game::buff& buff;
#else

#endif

public:
#ifndef BOT
    unbuff(const fb::game::buff& buff) :
        buff(buff)
    { }
#else
    unbuff() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string>(this->buff.model.name);
        writer.write<uint32_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
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
#ifndef BOT
    update(const fb::game::life& me, uint8_t index) :
        me(me),
        index(index)
    { }
#else
    update() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto spell = this->me.spells.at(index);
        if (spell == nullptr)
            co_return;

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1);
        writer.write<uint8_t>(static_cast<uint8_t>(spell->type));
        writer.write<std::string>(spell->name);

        if (static_cast<int>(spell->type) < 3)
            writer.write<std::string>(spell->message);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
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
#ifndef BOT
    const fb::game::life& me;
    const uint8_t         index;
#else

#endif

public:
#ifndef BOT
    remove(const fb::game::life& me, uint8_t index) :
        me(me),
        index(index)
    { }
#else
    remove() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto spell = this->me.spells.at(index);
        if (spell != nullptr)
            co_return;

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->index + 1);
        writer.write<uint8_t>(0x00);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

} // namespace fb::protocol::game::response::spell

#endif // !__PROTOCOL_RESPONSE_GAME_SPELL_H__