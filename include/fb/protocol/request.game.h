#ifndef __PROTOCOL_REQUEST_GAME_H__
#define __PROTOCOL_REQUEST_GAME_H__

#include <fb/protocol/protocol.h>
#include <fb/game/mmo.h>
#include <fb/game/trade.h>
#include <fb/game/dialog.h>
#include <optional>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request {

class login : public fb::protocol::base::header
{
public:
    struct transfer_param
    {
    public:
        uint16_t            map;
        point16_t           position;
    };

public:
    uint8_t                         enc_type;
    uint8_t                         key_size;
    uint8_t                         enc_key[0x09];
    internal::services              from;
    uint32_t                        id;
    std::string                     name;
    std::optional<transfer_param>   transfer;

public:
#ifndef BOT
    login() : fb::protocol::base::header(0x10)
    { }
#else
    login(const fb::buffer& params) : fb::protocol::base::header(0x10)
    {
        auto in_stream = fb::istream((const uint8_t*)params.data(), params.size());
        this->deserialize(in_stream);
    }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->enc_type)
                  .write_u8(this->key_size)
                  .write((void*)this->enc_key, this->key_size)
                  .write_u8(this->from)
                  .write_u32(this->id)
                  .writestr_u8(this->name)
                  .write_u8(this->transfer.has_value());

        if(transfer.has_value())
        {
            out_stream.write_u16(this->transfer.value().map)
                      .write_u16(this->transfer.value().position.x)
                      .write_u16(this->transfer.value().position.y);
        }
    }
#endif
    void deserialize(fb::istream& in_stream)
    {
        // base
        this->enc_type = in_stream.read_u8();
        this->key_size = in_stream.read_u8();
        in_stream.read((void*)this->enc_key, this->key_size);
        this->from = (fb::protocol::internal::services)in_stream.read_u8();

        // additional parameters
        this->id = in_stream.read_u32();
        this->name = in_stream.readstr_u8();

        auto transfer = in_stream.read_8();
        if(transfer == 1)
        {
            auto map = in_stream.read_u16();
            auto x = in_stream.read_u16();
            auto y = in_stream.read_u16();

            this->transfer = transfer_param
            {
                .map = map,
                .position = point16_t(x, y)
            };
        }
    }
};


class direction : public fb::protocol::base::header
{
public:
#ifndef BOT
    fb::game::direction         value;
#else
    const fb::game::direction   value;
#endif

public:
#ifndef BOT
    direction() : fb::protocol::base::header(0x11)
    { }
#else
    direction(fb::game::direction value) : fb::protocol::base::header(0x11), value(value)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8((uint8_t)this->value);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->value = fb::game::direction(in_stream.read_u8());
    }
#endif
};

class exit : public fb::protocol::base::header
{
public:
    exit() : fb::protocol::base::header(0x0B)
    { }

public:
    void deserialize(fb::istream& in_stream) { }
};


class move : public fb::protocol::base::header
{
public:
    fb::game::direction     direction;
    uint8_t                 sequence;
    point16_t               position;

protected:
    move(int id) : fb::protocol::base::header(id)
    { }

public:
#ifndef BOT
    move() : fb::protocol::base::header(0x32)
    { }
#else
    move(fb::game::direction direction, uint32_t sequence, point16_t position) : fb::protocol::base::header(0x32),
        direction(direction), sequence(sequence), position(position)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->direction);
        out_stream.write_u8(this->sequence);
        out_stream.write_u16(this->position.x);
        out_stream.write_u16(this->position.y);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->direction = fb::game::direction(in_stream.read_u8());
        this->sequence = in_stream.read_u8();
        this->position.x = in_stream.read_u16();
        this->position.y = in_stream.read_u16();
    }
#endif
};


class update_move : public move
{
public:
    point16_t               begin;
    size8_t                 size;
    uint16_t                crc;

public:
    update_move() : move(0x06)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        move::deserialize(in_stream);

        this->begin.x = in_stream.read_u16();
        this->begin.y = in_stream.read_u16();
        this->size.width = in_stream.read_u8();
        this->size.height = in_stream.read_u8();
        this->crc = in_stream.read_u16();
    }
};

class attack : public fb::protocol::base::header
{
public:
    attack() : fb::protocol::base::header(0x13)
    { }

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
    }
#else
    void deserialize(fb::istream& in_stream) { }
#endif
};


class pick_up : public fb::protocol::base::header
{
public:
#ifndef BOT
    bool                    boost;
#else
    const bool              boost;
#endif

public:
#ifndef BOT
    pick_up() : fb::protocol::base::header(0x07)
    { }
#else
    pick_up(bool boost) : fb::protocol::base::header(0x07),
        boost(boost)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->boost);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->boost = bool(in_stream.read_u8());
    }
#endif
};


class emotion : public fb::protocol::base::header
{
public:
#ifndef BOT
    uint8_t                 value;
#else
    const uint8_t           value;
#endif

public:
#ifndef BOT
    emotion() : fb::protocol::base::header(0x1D)
    { }
#else
    emotion(uint8_t value) : fb::protocol::base::header(0x1D),
        value(value)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->value);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->value = in_stream.read_u8();
    }
#endif
};


class refresh : public fb::protocol::base::header
{
public:
    refresh() : fb::protocol::base::header(0x38)
    { }

public:
    void deserialize(fb::istream& in_stream) { }
};

class front_info : public fb::protocol::base::header
{
public:
    front_info() : fb::protocol::base::header(0x09)
    { }

public:
    void deserialize(fb::istream& in_stream) { }
};

class self_info : public fb::protocol::base::header
{
public:
    self_info() : fb::protocol::base::header(0x2D)
    { }

public:
    void deserialize(fb::istream& in_stream) { }
};


class change_option : public fb::protocol::base::header
{
public:
    fb::game::options       option;

public:
    change_option() : fb::protocol::base::header(0x1B)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->option = fb::game::options(in_stream.read_u8());
    }
};


class click : public fb::protocol::base::header
{
public:
    uint32_t                fd;

public:
    click() : fb::protocol::base::header(0x43)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        auto unknown = in_stream.read_u8();
        this->fd = in_stream.read_u32();
    }
};


class trade : public fb::protocol::base::header
{
public:
    typedef union
    {
    public:
        uint8_t             index;
        uint16_t            count;
        uint32_t            money;
    } params;

public:
    uint8_t                 action;
    uint32_t                fd;
    params                  parameter;

public:
    trade() : fb::protocol::base::header(0x4A)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        this->fd = in_stream.read_u32();
        switch(static_cast<fb::game::trade::state>(this->action))
        {
        case fb::game::trade::state::UP_ITEM:
            this->parameter.index = in_stream.read_u8();
            break;

        case fb::game::trade::state::ITEM_COUNT:
            this->parameter.count = in_stream.read_u16();
            break;

        case fb::game::trade::state::UP_MONEY:
            this->parameter.money = in_stream.read_u32();
            break;
        }
    }
};


class group : public fb::protocol::base::header
{
public:
    std::string             name;

public:
    group() : fb::protocol::base::header(0x2E)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};


class user_list : public fb::protocol::base::header
{
public:
    user_list() : fb::protocol::base::header(0x18)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        auto unknown = in_stream.read_u8();
    }
};


class chat : public fb::protocol::base::header
{
public:
#ifndef BOT
    bool                    shout;
    std::string             message;
#else
    const bool              shout;
    const std::string       message;
#endif

public:
#ifndef BOT
    chat() : fb::protocol::base::header(0x0E)
    { }
#else
    chat(bool shout, const std::string& message) : fb::protocol::base::header(0x0E),
        shout(shout), message(message)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->shout)
                  .writestr_u8(this->message);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->shout = in_stream.read_u8();
        this->message = in_stream.readstr_u8();
    }
#endif
};


class swap : public fb::protocol::base::header
{
public:
    fb::game::swap::type    type;
    uint8_t                 src;
    uint8_t                 dst;

public:
    swap() : fb::protocol::base::header(0x30)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->type = fb::game::swap::type(in_stream.read_u8());
        this->src = in_stream.read_u8();
        this->dst = in_stream.read_u8();
    }
};


class dialog : public fb::protocol::base::header
{
public:
    fb::game::dialog::interaction   interaction;
    uint8_t                         action;     // NORMAL
    std::string                     message;    // INPUT
    uint16_t                        index;      // MENU
    uint16_t                        pursuit;    // SELL
    std::string                     name;       // SELL

protected:
    dialog(int id) : fb::protocol::base::header(id)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->interaction = static_cast<fb::game::dialog::interaction>(in_stream.read_u8());
        switch(static_cast<fb::game::dialog::interaction>(this->interaction))
        {
        case fb::game::dialog::interaction::NORMAL: // 일반 다이얼로그
        {
            in_stream.read(nullptr, 0x07); // 7바이트 무시
            this->action = in_stream.read_u8();
            break;
        }

        case fb::game::dialog::interaction::INPUT:
        {
            auto unknown1 = in_stream.read_u16();
            auto unknown2 = in_stream.read_u32();
            this->message = in_stream.readstr_u16();
            break;
        }

        case fb::game::dialog::interaction::INPUT_EX:
        {
            in_stream.read(nullptr, 0x07); // 7바이트 무시
            this->action = in_stream.read_u8();
            if(this->action == 0x02) // OK button
            {
                auto unknown1 = in_stream.read_u8();
                this->message = in_stream.readstr_u8();
            }
            break;
        }

        case fb::game::dialog::interaction::MENU:
        {
            auto unknown = in_stream.read_u32();
            this->index = in_stream.read_u16();
            break;
        }

        case fb::game::dialog::interaction::SELL:
        {
            auto unknown = in_stream.read_u32();
            this->pursuit = in_stream.read_u16();
            this->name = in_stream.readstr_u8();
            break;
        }
        }
    }
};

class dialog1 : public dialog
{
public:
    dialog1() : dialog(0x3A)
    { }
};

class dialog2 : public dialog
{
public:
    dialog2() : dialog(0x39)
    { }
};


class door : public fb::protocol::base::header
{
public:
    door() : fb::protocol::base::header(0x20)
    { }

public:
    void deserialize(fb::istream& in_stream)
    { }
};


class whisper : public fb::protocol::base::header
{
public:
    std::string             name;
    std::string             message;

public:
    whisper() : fb::protocol::base::header(0x19)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->message = in_stream.readstr_u8();
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_GAME_H__