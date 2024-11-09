#ifndef __PROTOCOL_REQUEST_GAME_H__
#define __PROTOCOL_REQUEST_GAME_H__

#include <dialog.h>
#include <fb/protocol/protocol.h>
#include <mmo.h>
#include <optional>
#include <trade.h>

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request {

class login : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x10;

public:
    struct transfer_param
    {
    public:
        uint16_t  map;
        point16_t position;
    };

public:
    uint8_t                       enc_type;
    uint8_t                       key_size;
    uint8_t                       enc_key[0x09];
    internal::services            from;
    uint32_t                      id;
    std::string                   name;
    std::optional<transfer_param> transfer;

public:
#ifndef BOT
    login() = default;
#else
    login(const fb::buffer& params)
    {
        auto in_stream = fb::istream((const uint8_t*)params.data(), params.size());
        this->deserialize(in_stream);
    }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(this->enc_type);
        out_stream.write_u8(this->key_size);
        out_stream.write((void*)this->enc_key, this->key_size);
        out_stream.write_u8(this->from);
        out_stream.write_u32(this->id);
        out_stream.writestr_u8(this->name);
        out_stream.write_u8(this->transfer.has_value());

        if (transfer.has_value())
        {
            out_stream.write_u16(this->transfer.value().map);
            out_stream.write_u16(this->transfer.value().position.x);
            out_stream.write_u16(this->transfer.value().position.y);
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
        this->id   = in_stream.read_u32();
        this->name = in_stream.readstr_u8();

        auto transfer = in_stream.read_8();
        if (transfer == 1)
        {
            auto map       = in_stream.read_u16();
            auto x         = in_stream.read_u16();
            auto y         = in_stream.read_u16();
            this->transfer = transfer_param{.map = map, .position = point16_t(x, y)};
        }
    }
};

class direction : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x11;

public:
#ifndef BOT
    DIRECTION value;
#else
    const DIRECTION value;
#endif

public:
#ifndef BOT
    direction() = default;
#else
    direction(DIRECTION value) :
        value(value)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8((uint8_t)this->value);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->value = DIRECTION(in_stream.read_u8());
    }
#endif
};

class exit : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0B;

public:
    exit() = default;

public:
    void deserialize(fb::istream& in_stream)
    { }
};

class move : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x32;

public:
    DIRECTION direction;
    uint8_t   sequence;
    point16_t position;

public:
#ifndef BOT
    move() = default;
#else
    move(DIRECTION direction, uint32_t sequence, point16_t position) :
        direction(direction),
        sequence(sequence),
        position(position)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(this->direction);
        out_stream.write_u8(this->sequence);
        out_stream.write_u16(this->position.x);
        out_stream.write_u16(this->position.y);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->direction  = DIRECTION(in_stream.read_u8());
        this->sequence   = in_stream.read_u8();
        this->position.x = in_stream.read_u16();
        this->position.y = in_stream.read_u16();
    }
#endif
};

class update_move : public move
{
public:
    inline static uint8_t header = 0x06;

public:
    point16_t begin;
    size8_t   size;
    uint16_t  crc;

public:
    update_move() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        move::deserialize(in_stream);

        this->begin.x     = in_stream.read_u16();
        this->begin.y     = in_stream.read_u16();
        this->size.width  = in_stream.read_u8();
        this->size.height = in_stream.read_u8();
        this->crc         = in_stream.read_u16();
    }
};

class attack : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x13;

public:
    attack() = default;

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
    }
#else
    void deserialize(fb::istream& in_stream)
    { }
#endif
};

class pick_up : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x07;

public:
#ifndef BOT
    bool boost;
#else
    const bool boost;
#endif

public:
#ifndef BOT
    pick_up() = default;
#else
    pick_up(bool boost) :
        boost(boost)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
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
    inline static uint8_t header = 0x1D;

public:
#ifndef BOT
    uint8_t value;
#else
    const uint8_t value;
#endif

public:
#ifndef BOT
    emotion() = default;
#else
    emotion(uint8_t value) :
        value(value)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
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
    inline static uint8_t header = 0x38;

public:
    refresh() = default;

public:
    void deserialize(fb::istream& in_stream)
    { }
};

class front_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x09;

public:
    front_info() = default;

public:
    void deserialize(fb::istream& in_stream)
    { }
};

class self_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2D;

public:
    self_info() = default;

public:
    void deserialize(fb::istream& in_stream)
    { }
};

class change_option : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1B;

public:
    CUSTOM_SETTING option;

public:
    change_option() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->option = CUSTOM_SETTING(in_stream.read_u8());
    }
};

class click : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x43;

public:
    uint32_t fd;

public:
    click() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        auto unknown = in_stream.read_u8();
        this->fd     = in_stream.read_u32();
    }
};

class trade : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x4A;

public:
    typedef union
    {
    public:
        uint8_t  index;
        uint16_t count;
        uint32_t money;
    } params;

public:
    uint8_t  action;
    uint32_t fd;
    params   parameter;

public:
    trade() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        this->fd     = in_stream.read_u32();
        switch (static_cast<fb::game::trade::state>(this->action))
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
    inline static uint8_t header = 0x2E;

public:
    std::string name;

public:
    group() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};

class user_list : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x18;

public:
    user_list() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        auto unknown = in_stream.read_u8();
    }
};

class chat : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x0E;

public:
#ifndef BOT
    bool        shout;
    std::string message;
#else
    const bool        shout;
    const std::string message;
#endif

public:
#ifndef BOT
    chat() = default;
#else
    chat(bool shout, const std::string& message) :
        fb::protocol::base::header(0x0E),
        shout(shout),
        message(message)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(this->shout).writestr_u8(this->message);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->shout   = in_stream.read_u8();
        this->message = in_stream.readstr_u8();
    }
#endif
};

class swap : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x30;

public:
    SWAP_TYPE type;
    uint8_t   src;
    uint8_t   dst;

public:
    swap() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->type = SWAP_TYPE(in_stream.read_u8());
        this->src  = in_stream.read_u8();
        this->dst  = in_stream.read_u8();
    }
};

class dialog : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3A;

public:
    fb::game::dialog::interaction interaction;
    uint8_t                       action;  // NORMAL
    std::string                   message; // INPUT
    uint16_t                      index;   // MENU
    uint16_t                      pursuit; // SELL
    std::string                   name;    // SELL

public:
    dialog() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->interaction = static_cast<fb::game::dialog::interaction>(in_stream.read_u8());
        switch (static_cast<fb::game::dialog::interaction>(this->interaction))
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
            if (this->action == 0x02) // OK button
            {
                auto unknown1 = in_stream.read_u8();
                this->message = in_stream.readstr_u8();
            }
            break;
        }

        case fb::game::dialog::interaction::MENU:
        {
            auto unknown = in_stream.read_u32();
            this->index  = in_stream.read_u16();
            break;
        }

        case fb::game::dialog::interaction::ITEM:
        {
            auto unknown  = in_stream.read_u32();
            this->pursuit = in_stream.read_u16();
            this->name    = in_stream.readstr_u8();
            break;
        }

        case fb::game::dialog::interaction::SLOT:
        {
            auto unknown  = in_stream.read_u32();
            this->pursuit = in_stream.read_u16();
            this->index   = in_stream.read_u8();
            break;
        }
        }
    }
};

class door : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x20;

public:
    door() = default;

public:
    void deserialize(fb::istream& in_stream)
    { }
};

class whisper : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x19;

public:
    std::string name;
    std::string message;

public:
    whisper() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->name    = in_stream.readstr_u8();
        this->message = in_stream.readstr_u8();
    }
};

}}}} // namespace fb::protocol::game::request

#endif // !__PROTOCOL_REQUEST_GAME_H__