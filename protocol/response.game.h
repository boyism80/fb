#ifndef __PROTOCOL_RESPONSE_GAME_H__
#define __PROTOCOL_RESPONSE_GAME_H__

#include "protocol/protocol.h"
#include "model/session/session.h"

namespace fb { namespace protocol { namespace response { namespace game {

class init : public fb::protocol::base::response
{
public:
    init()
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x1E)
                  .write_u8(0x06)
                  .write_u8(0x00);
    }
};

class message : public fb::protocol::base::response
{
public:
    const std::string                       text;
    const fb::game::message::type           type;

public:
    message(const std::string& text, fb::game::message::type type) : 
        text(text), type(type)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x0A)
                  .write_u8(this->type)
                  .write(this->text, true);
    }
};

class user_list : public fb::protocol::base::response
{
private:
    typedef fb::base::socket_container<fb::socket, fb::game::session> container;

public:
    const fb::game::session&                me;
    const container&                        sockets;

public:
    user_list(const fb::game::session& me, const container& sockets) : 
        me(me), sockets(sockets)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x36)
                  .write_u16((uint16_t)sockets.size())
                  .write_u16((uint16_t)sockets.size())
                  .write_u8(0x00);

        for(auto& socket : this->sockets)
        {
            auto user = socket->data();
            auto& name = user->name();

            out_stream.write_u8(0x10 * user->nation())
                      .write_u8(0x10 * user->promotion())
                      .write_u8((&this->me == user) ? 0x88 : 0x0F)
                      .write(name, false);
        }
    }
};

class chat : public fb::protocol::base::response
{
public:
    const fb::game::object&     me;
    const std::string           text;
    const fb::game::chat::type  type;

public:
    chat(const fb::game::object& me, const std::string& text, fb::game::chat::type type) : 
        me(me), text(text), type(type)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x0D)
                  .write_u8(this->type)
                  .write_u32(this->me.sequence())
                  .write(this->text);
    }
};

class time : public fb::protocol::base::response
{
public:
    const uint8_t               hours;

public:
    time(uint8_t hours) : 
        hours(hours)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x20)      // cmd : 0x20
                  .write_u8(this->hours%24)  // hours
                  .write_u8(0x00)      // Unknown
                  .write_u8(0x00);     // Unknown
    }
};

class weather : public fb::protocol::base::response
{
public:
    const fb::game::weather::type   value;

public:
    weather(fb::game::weather::type value) : 
        value(value)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x1F)
                  .write_u8(this->value)
                  .write_u8(0x00);
    }
};

class bright : public fb::protocol::base::response
{
public:
    const uint8_t                   value;

public:
    bright(uint8_t value) : 
        value(value)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x20)
                  .write_u8(0x00)
                  .write_u8(std::max(0, 20 - this->value));
    }
};

class timer : public fb::protocol::base::response
{
public:
    const uint32_t                  time;
    const fb::game::timer::type     type;

public:
    timer(uint32_t time, fb::game::timer::type type = fb::game::timer::type::DECREASE) : 
        time(time), type(type)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x67)
                  .write_u8(this->type)
                  .write_u32(this->time)
                  .write_u8(0x00);
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_GAME_H__