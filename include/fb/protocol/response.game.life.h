#ifndef __PROTOCOL_RESPONSE_GAME_LIFE_H__
#define __PROTOCOL_RESPONSE_GAME_LIFE_H__

#include <fb/protocol/protocol.h>
#ifndef BOT
#include <life.h>
#endif

#ifndef BOT
using namespace fb::game;
#endif

namespace fb::protocol::game::response::life {

class action : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1A;

public:
#ifndef BOT
    const fb::game::life& me;
    const ACTION          value;
    const DURATION        duration;
    const uint8_t         sound;
#else
    uint32_t sequence;
    ACTION   value;
    DURATION duration;
    uint8_t  sound;
#endif

public:
#ifndef BOT
    action(const fb::game::life& me, ACTION value, DURATION duration, uint8_t sound = 0x00) :
        me(me),
        value(value),
        duration(duration),
        sound(sound)
    { }
#else
    action() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint8_t>(static_cast<uint8_t>(this->value));      // type
        writer.write<uint16_t>(static_cast<uint16_t>(this->duration)); // duration
        writer.write<uint8_t>(this->sound);                            // sound
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->sequence = reader.read<uint32_t>();
        this->value    = (ACTION)reader.read<uint8_t>();
        this->duration = (DURATION)reader.read<uint16_t>();
        this->sound    = reader.read<uint8_t>();
    }
#endif
};

class show_hp : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x13;

public:
#ifndef BOT
    const fb::game::life& me;
    const uint32_t        damage;
    const bool            critical;
    const uint8_t         percentage;
#else

#endif

public:
#ifndef BOT
    show_hp(const fb::game::life& me, uint32_t damage, bool critical) :
        me(me),
        damage(damage),
        critical(critical),
        percentage(static_cast<uint8_t>(this->me.hp() / float(this->me.base_hp()) * 100))
    { }
#else
    show_hp() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->me.sequence());
        writer.write<uint8_t>(this->critical);
        writer.write<uint8_t>(this->percentage);
        writer.write<uint32_t>(this->damage);
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

class die : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x5F;

public:
#ifndef BOT
    const uint32_t id;
#else

#endif

public:
#ifndef BOT
    die(const fb::game::life& life) :
        die(life.sequence())
    { }
    die(uint32_t id) :
        id(id)
    { }
#else
    die() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint32_t>(this->id);
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

} // namespace fb::protocol::game::response::life

#endif // !__PROTOCOL_RESPONSE_GAME_LIFE_H__