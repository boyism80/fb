#ifndef __PROTOCOL_RESPONSE_GAME_H__
#define __PROTOCOL_RESPONSE_GAME_H__

#include <protocol/protocol.h>
#include <model/session/session.h>

namespace fb { namespace protocol { namespace response { namespace game {

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
public:
    const fb::game::session&                        me;
    const session_container<fb::game::session>&     users;

public:
    user_list(const fb::game::session& me, const session_container<fb::game::session>& users) : 
        me(me), users(users)
    {}

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x36)
                  .write_u16((uint16_t)users.size())
                  .write_u16((uint16_t)users.size())
                  .write_u8(0x00);

        for(const auto& i : this->users)
        {
            const auto& name = i->name();

            out_stream.write_u8(0x10 * i->nation())
                      .write_u8(0x10 * i->promotion())
                      .write_u8((&this->me == i) ? 0x88 : 0x0F)
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
                  .write_u32(this->me.id())
                  .write(this->text);
    }
};

} } } }

#endif // !__PROTOCOL_RESPONSE_GAME_H__