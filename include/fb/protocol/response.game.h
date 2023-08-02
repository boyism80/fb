#ifndef __PROTOCOL_RESPONSE_GAME_H__
#define __PROTOCOL_RESPONSE_GAME_H__

#include <fb/protocol/protocol.h>
#include <fb/game/session.h>

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
};

class message : public fb::protocol::base::header
{
public:
    const std::string                       text;
    const fb::game::message::type           type;

public:
    message(const std::string& text, fb::game::message::type type) : fb::protocol::base::header(0x0A),
        text(text), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write(this->text, true);
    }
};

class user_list : public fb::protocol::base::header
{
    
private:
    using container = fb::base::socket_container<fb::socket, fb::game::session>;

public:
    const fb::game::session&                me;
    const container&                        sockets;

public:
    user_list(const fb::game::session& me, const container& sockets) : fb::protocol::base::header(0x36),
        me(me), sockets(sockets)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u16((uint16_t)sockets.size())
                  .write_u16((uint16_t)sockets.size())
                  .write_u8(0x00);

        for(auto& [key, value] : this->sockets)
        {
            auto user = value->data();
            auto& name = user->name();

            out_stream.write_u8(0x10 * static_cast<int>(user->nation()))
                      .write_u8(0x10 * static_cast<int>(user->promotion()))
                      .write_u8((&this->me == user) ? 0x88 : 0x0F)
                      .write(name, false);
        }
    }
};

class chat : public fb::protocol::base::header
{
public:
    const fb::game::object&     me;
    const std::string           text;
    const fb::game::chat::type  type;

public:
    chat(const fb::game::object& me, const std::string& text, fb::game::chat::type type) : fb::protocol::base::header(0x0D),
        me(me), text(text), type(type)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->type)
                  .write_u32(this->me.sequence())
                  .write(this->text);
    }
};

class time : public fb::protocol::base::header
{
public:
    const uint8_t               hours;

public:
    time(uint8_t hours) : fb::protocol::base::header(0x20),
        hours(hours)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->hours%24)  // hours
                  .write_u8(0x00)      // Unknown
                  .write_u8(0x00);     // Unknown
    }
};

class weather : public fb::protocol::base::header
{
public:
    const fb::game::weather::type   value;

public:
    weather(fb::game::weather::type value) : fb::protocol::base::header(0x1F),
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
    const fb::game::timer::type     type;

public:
    timer(uint32_t time, fb::game::timer::type type = fb::game::timer::type::DECREASE) : fb::protocol::base::header(0x67),
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