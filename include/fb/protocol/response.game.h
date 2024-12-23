#ifndef __PROTOCOL_RESPONSE_GAME_H__
#define __PROTOCOL_RESPONSE_GAME_H__

#include <fb/protocol/protocol.h>
#ifndef BOT
#include <character.h>
#endif

namespace fb::protocol::game::response {

class init : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1E;

public:
    init() = default;

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x06);
        writer.write<uint8_t>(0x00);
    }
#ifdef BOT
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        reader.read<uint8_t>();
        reader.read<uint8_t>();
    }
#endif
};

class message : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0A;

public:
#ifdef BOT
    std::string  text;
    MESSAGE_TYPE type;
#else
    const std::string  text;
    const MESSAGE_TYPE type;
#endif

public:
#ifdef BOT
    message() = default;
#else
    message(const std::string& text, MESSAGE_TYPE type) :
        text(text),
        type(type)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<std::string, uint16_t>(this->text);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->type = (MESSAGE_TYPE)reader.read<uint8_t>();
        this->text = reader.read<std::string, uint16_t>();
    }
#endif
};

class user_list : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x36;

private:
#ifdef BOT
#else
    using container = fb::socket_container<fb::game::character>;
#endif

public:
#ifdef BOT
#else
    const fb::game::character& me;
    container&                 sockets;
#endif

public:
#ifdef BOT
    user_list()
    { }
#else
    user_list(const fb::game::character& me, container& sockets) :
        me(me),
        sockets(sockets)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint16_t>((uint16_t)sockets.size());
        writer.write<uint16_t>((uint16_t)sockets.size());
        writer.write<uint8_t>(0x00);

        co_await this->sockets.each([this, &writer](auto& socket) -> async::task<void> {
            auto  user = socket.data();
            auto& name = user->name();

            writer.write<uint8_t>(0x10 * static_cast<int>(user->nation()));
            writer.write<uint8_t>(0x10 * static_cast<int>(user->promotion()));
            writer.write<uint8_t>((&this->me == user) ? 0x88 : 0x0F);
            writer.write<std::string, uint8_t>(name);
            co_return;
        });
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        // TODO: deserialize bytes
    }
#endif
};

class chat : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0D;

public:
#ifndef BOT
    const fb::game::object& me;
    const std::string       text;
    const CHAT_TYPE         type;
#else
    uint32_t    sequence;
    std::string text;
    CHAT_TYPE   type;
#endif

public:
#ifndef BOT
    chat(const fb::game::object& me, const std::string& text, CHAT_TYPE type) :
        me(me),
        text(text),
        type(type)
    { }
#else
    chat() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<uint32_t>(this->me.sequence());
        writer.write<std::string>(this->text);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->type     = (CHAT_TYPE)reader.read<uint8_t>();
        this->sequence = reader.read<uint32_t>();
        this->text     = reader.read<std::string, uint8_t>();
    }
#endif
};

class time : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x20;

public:
#ifdef BOT
    uint8_t hours;
#else
    const uint8_t hours;
#endif

public:
#ifdef BOT
    time() = default;
#else
    time(uint8_t hours) :
        hours(hours)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->hours % 24); // hours
        writer.write<uint8_t>(0x00);             // Unknown
        writer.write<uint8_t>(0x00);             // Unknown
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->hours = reader.read<uint8_t>();
        reader.read<uint8_t>();
        reader.read<uint8_t>();
    }
#endif
};

class weather : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1F;

public:
    const WEATHER_TYPE value;

public:
    weather(WEATHER_TYPE value) :
        value(value)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->value));
        writer.write<uint8_t>(0x00);
    }
};

class bright : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x20;

public:
    const uint8_t value;

public:
    bright(uint8_t value) :
        value(value)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x00);
        writer.write<uint8_t>(std::max(0, 20 - this->value));
    }
};

class timer : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x67;

public:
    const uint32_t   time;
    const TIMER_TYPE type;

public:
    timer(uint32_t time, TIMER_TYPE type = TIMER_TYPE::DECREASE) :
        time(time),
        type(type)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(static_cast<uint8_t>(this->type));
        writer.write<uint32_t>(this->time);
        writer.write<uint8_t>(0x00);
    }
};

} // namespace fb::protocol::game::response

#endif // !__PROTOCOL_RESPONSE_GAME_H__