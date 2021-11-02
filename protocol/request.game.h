#ifndef __PROTOCOL_REQUEST_GAME_H__
#define __PROTOCOL_REQUEST_GAME_H__

#include "protocol/protocol.h"
#include "model/trade/trade.h"

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request {

class login : public fb::protocol::base::header
{
public:
    uint8_t                 enc_type;
    uint8_t                 key_size;
    uint8_t                 enc_key[0x09];
    internal::services      from;
    std::string             name;

public:
    void deserialize(fb::istream& in_stream)
    {
        // base
        this->enc_type = in_stream.read_u8();
        this->key_size = in_stream.read_u8();
        in_stream.read((void*)this->enc_key, this->key_size);
        this->from = (fb::protocol::internal::services)in_stream.read_u8();
        
        // additional parameters
        this->name = in_stream.readstr_u8();
    }
};


class direction : public fb::protocol::base::header
{
public:
    fb::game::direction     direction;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->direction = fb::game::direction(in_stream.read_u8());
    }
};

class exit : public fb::protocol::base::header
{
public:
    void deserialize(fb::istream& in_stream) { }
};


class move : public fb::protocol::base::header
{
public:
    fb::game::direction     direction;
    uint8_t                 sequence;
    point16_t               position;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->direction = fb::game::direction(in_stream.read_u8());
        this->sequence = in_stream.read_u8();
        this->position.x = in_stream.read_u16();
        this->position.y = in_stream.read_u16();
    }
};


class update_move : public fb::protocol::game::request::move
{
public:
    point16_t               begin;
    size8_t                 size;
    uint16_t                crc;

public:
    void deserialize(fb::istream& in_stream)
    {
        fb::protocol::game::request::move::deserialize(in_stream);

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
    void deserialize(fb::istream& in_stream) { }
};


class pick_up : public fb::protocol::base::header
{
public:
    bool                    boost;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->boost = bool(in_stream.read_u8());
    }
};


class emotion : public fb::protocol::base::header
{
public:
    uint8_t                 emotion;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->emotion = in_stream.read_u8();
    }
};


class refresh : public fb::protocol::base::header
{
public:
    void deserialize(fb::istream& in_stream) { }
};

class front_info : public fb::protocol::base::header
{
public:
    void deserialize(fb::istream& in_stream) { }
};

class self_info : public fb::protocol::base::header
{
public:
    void deserialize(fb::istream& in_stream) { }
};


class change_option : public fb::protocol::base::header
{
public:
    fb::game::options       option;

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
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        this->fd = in_stream.read_u32();
        switch(action)
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
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
    }
};


class user_list : public fb::protocol::base::header
{
public:
    void deserialize(fb::istream& in_stream)
    {
        auto unknown = in_stream.read_u8();
    }
};


class chat : public fb::protocol::base::header
{
public:
    bool                    shout;
    std::string             message;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->shout = in_stream.read_u8();
        this->message = in_stream.readstr_u8();
    }
};


class swap : public fb::protocol::base::header
{
public:
    fb::game::swap::type    type;
    uint8_t                 src;
    uint8_t                 dst;

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
    uint8_t                 interaction;
    uint8_t                 action;     // NORMAL
    std::string             message;    // INPUT
    uint16_t                index;      // MENU
    uint16_t                pursuit;    // SELL
    std::string             name;       // SELL

public:
    void deserialize(fb::istream& in_stream)
    {
        this->interaction = in_stream.read_u8();
        switch(this->interaction)
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


class door : public fb::protocol::base::header
{
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
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->message = in_stream.readstr_u8();
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_GAME_H__