#ifndef __PROTOCOL_RESPONSE_GAME_H__
#define __PROTOCOL_RESPONSE_GAME_H__

#include <fb/protocol/protocol.h>
#include <session.h>

namespace fb { namespace protocol { namespace game { namespace response {

class init : public fb::protocol::base::header
{
public:
    init() : fb::protocol::base::header(0x1E)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x06)
                  .write_u8(0x00);
    }
#ifdef BOT
    void deserialize(fb::istream& in_stream)
    {
        in_stream.read_u8();
        in_stream.read_u8();
    }
#endif
};

class message : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::string                             text;
    MESSAGE_TYPE                  type;
#else
    const std::string                       text;
    const MESSAGE_TYPE            type;
#endif

public:
#ifdef BOT
    message() : fb::protocol::base::header(0x0A)
    { }
#else
    message(const std::string& text, MESSAGE_TYPE type) : fb::protocol::base::header(0x0A),
        text(text), type(type)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write(this->text, true);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->type = (MESSAGE_TYPE)in_stream.read_u8();
        this->text = in_stream.readstr_u16();
    }
#endif
};

class user_list : public fb::protocol::base::header
{
private:
    using container = fb::socket_container<fb::game::session>;

public:
    const fb::game::session&                me;
    container&                              sockets;

public:
    user_list(const fb::game::session& me, container& sockets) : fb::protocol::base::header(0x36),
        me(me), sockets(sockets)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u16((uint16_t)sockets.size())
                  .write_u16((uint16_t)sockets.size())
                  .write_u8(0x00);

        this->sockets.each([this, &out_stream] (auto& socket)
        {
            auto user = socket.data();
            auto& name = user->name();

            out_stream.write_u8(0x10 * static_cast<int>(user->nation()))
                      .write_u8(0x10 * static_cast<int>(user->promotion()))
                      .write_u8((&this->me == user) ? 0x88 : 0x0F)
                      .write(name, false);
        });
    }
};

class chat : public fb::protocol::base::header
{
public:
#ifndef BOT
    const fb::game::object&     me;
    const std::string           text;
    const CHAT_TYPE   type;
#else
    uint32_t                    sequence;
    std::string                 text;
    CHAT_TYPE         type;
#endif

public:
#ifndef BOT
    chat(const fb::game::object& me, const std::string& text, CHAT_TYPE type) : fb::protocol::base::header(0x0D),
        me(me), text(text), type(type)
    { }
#else
    chat() : fb::protocol::base::header(0x0D)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write_u32(this->me.sequence())
                  .write(this->text);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->type = (CHAT_TYPE)in_stream.read_u8();
        this->sequence = in_stream.read_u32();
        this->text = in_stream.readstr_u8();
    }
#endif
};

class time : public fb::protocol::base::header
{
public:
#ifdef BOT
    uint8_t                     hours;
#else
    const uint8_t               hours;
#endif

public:
#ifdef BOT
    time() : fb::protocol::base::header(0x20)
    { }
#else
    time(uint8_t hours) : fb::protocol::base::header(0x20),
        hours(hours)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->hours%24)  // hours
                  .write_u8(0x00)      // Unknown
                  .write_u8(0x00);     // Unknown
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->hours = in_stream.read_u8();
        in_stream.read_u8();
        in_stream.read_u8();
    }
#endif
};

class weather : public fb::protocol::base::header
{
public:
    const WEATHER_TYPE    value;

public:
    weather(WEATHER_TYPE value) : fb::protocol::base::header(0x1F),
        value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->value)
                  .write_u8(0x00);
    }
};

class bright : public fb::protocol::base::header
{
public:
    const uint8_t                   value;

public:
    bright(uint8_t value) : fb::protocol::base::header(0x20),
        value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x00)
                  .write_u8(std::max(0, 20 - this->value));
    }
};

class timer : public fb::protocol::base::header
{
public:
    const uint32_t                  time;
    const TIMER_TYPE      type;

public:
    timer(uint32_t time, TIMER_TYPE type = TIMER_TYPE::DECREASE) : fb::protocol::base::header(0x67),
        time(time), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write_u32(this->time)
                  .write_u8(0x00);
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_GAME_H__