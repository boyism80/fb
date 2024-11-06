#ifndef __PROTOCOL_RESPONSE_GAME_TRADE_H__
#define __PROTOCOL_RESPONSE_GAME_TRADE_H__

#include <fb/protocol/protocol.h>
#include <trade.h>
#include <model.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace trade {

class dialog : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    const fb::game::character& me;
    const fb::model::model&    model;

public:
    dialog(const fb::game::character& me, const fb::model::model& model) :
        me(me),
        model(model)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto& cname = model.promotion[me.cls()][me.promotion()].name;

        std::stringstream sstream;
        sstream << this->me.name() << '(' << cname.c_str() << ')';

        out_stream.write_u8(header);
        out_stream.write_u8(0x00).write_u32(this->me.sequence()).write(sstream.str()).write_u8(0x00);
    }
};

class upload : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    const fb::game::character& me;
    const uint8_t              index;
    const bool                 mine;

public:
    upload(const fb::game::character& me, uint8_t index, bool mine) :
        me(me),
        index(index),
        mine(mine)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto item = this->me.trade.item(this->index);

        out_stream.write_u8(header);
        out_stream.write_u8(0x02)
            .write_u8(this->mine ? 0x00 : 0x01)
            .write_u8(this->index) // trade slot index
            .write_u16(item->look())
            .write_u8(item->color())
            .write(item->trade_name())
            .write_u8(0x00);
    }
};

class bundle : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    bundle() = default;

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(0x01).write_u8(0x00);
    }
};

class money : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    const fb::game::character& me;
    const bool                 mine;

public:
    money(const fb::game::character& me, bool mine) :
        me(me),
        mine(mine)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(0x03).write_u8(this->mine ? 0x00 : 0x01).write_u32(this->me.trade.money()).write_u8(0x00);
    }
};

class close : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    const std::string& message;

public:
    close(const std::string& message) :
        message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(0x04).write(this->message, true).write_u8(0x00);
    }
};

class lock : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x42;

public:
    lock() = default;

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(0x05).write_u8(0x00);
    }
};

}}}}} // namespace fb::protocol::game::response::trade

#endif // !__PROTOCOL_RESPONSE_GAME_TRADE_H__