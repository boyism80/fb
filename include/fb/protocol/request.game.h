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
    login(const fb::stream& params)
    {
        auto reader = fb::stream_reader<big_endian>((const uint8_t*)params.data(), params.size());
        this->deserialize(reader);
    }
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->enc_type);
        writer.write<uint8_t>(this->key_size);
        writer.write((void*)this->enc_key, this->key_size);
        writer.write<uint8_t>(this->from);
        writer.write<uint32_t>(this->id);
        writer.write<std::string, uint8_t>(this->name);
        writer.write<uint8_t>(this->transfer.has_value());

        if (transfer.has_value())
        {
            writer.write<uint16_t>(this->transfer.value().map);
            writer.write<uint16_t>(this->transfer.value().position.x);
            writer.write<uint16_t>(this->transfer.value().position.y);
        }
    }
#endif
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        // base
        this->enc_type = reader.read<uint8_t>();
        this->key_size = reader.read<uint8_t>();
        reader.read((void*)this->enc_key, this->key_size);
        this->from = (fb::protocol::internal::services)reader.read<uint8_t>();

        // additional parameters
        this->id      = reader.read<uint32_t>();
        this->name    = reader.read<std::string, uint8_t>();
        auto transfer = reader.read<uint8_t>();
        if (transfer == 1)
        {
            auto map       = reader.read<uint16_t>();
            auto x         = reader.read<uint16_t>();
            auto y         = reader.read<uint16_t>();
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>((uint8_t)this->value);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->value = DIRECTION(reader.read<uint8_t>());
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
    void deserialize(fb::stream_reader<big_endian>& reader)
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->direction);
        writer.write<uint8_t>(this->sequence);
        writer.write<uint16_t>(this->position.x);
        writer.write<uint16_t>(this->position.y);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->direction  = DIRECTION(reader.read<uint8_t>());
        this->sequence   = reader.read<uint8_t>();
        this->position.x = reader.read<uint16_t>();
        this->position.y = reader.read<uint16_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        move::deserialize(reader);

        this->begin.x     = reader.read<uint16_t>();
        this->begin.y     = reader.read<uint16_t>();
        this->size.width  = reader.read<uint8_t>();
        this->size.height = reader.read<uint8_t>();
        this->crc         = reader.read<uint16_t>();
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->boost);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->boost = bool(reader.read<uint8_t>());
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->value);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->value = reader.read<uint8_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

class front_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x09;

public:
    front_info() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

class self_info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x2D;

public:
    self_info() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    { }
};

class change_option : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1B;

public:
    SETTING option;

public:
    change_option() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->option = SETTING(reader.read<uint8_t>());
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        auto unknown = reader.read<uint8_t>();
        this->fd     = reader.read<uint32_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->action = reader.read<uint8_t>();
        this->fd     = reader.read<uint32_t>();
        switch (static_cast<fb::game::trade::state>(this->action))
        {
        case fb::game::trade::state::UP_ITEM:
            this->parameter.index = reader.read<uint8_t>();
            break;

        case fb::game::trade::state::ITEM_COUNT:
            this->parameter.count = reader.read<uint16_t>();
            break;

        case fb::game::trade::state::UP_MONEY:
            this->parameter.money = reader.read<uint32_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->name = reader.read<std::string, uint8_t>();
    }
};

class user_list : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x18;

public:
    user_list() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        auto unknown = reader.read<uint8_t>();
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
    void serialize(fb::stream_writer<big_endian>& writer) const
    {
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->shout);
        writer.write<std::string, uint8_t>(this->message);
    }
#else
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->shout   = reader.read<uint8_t>();
        this->message = reader.read<std::string, uint8_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->type = SWAP_TYPE(reader.read<uint8_t>());
        this->src  = reader.read<uint8_t>();
        this->dst  = reader.read<uint8_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->interaction = static_cast<fb::game::dialog::interaction>(reader.read<uint8_t>());
        switch (static_cast<fb::game::dialog::interaction>(this->interaction))
        {
        case fb::game::dialog::interaction::NORMAL: // 일반 다이얼로그
        {
            reader.read(nullptr, 0x07); // 7바이트 무시
            this->action = reader.read<uint8_t>();
            break;
        }

        case fb::game::dialog::interaction::INPUT:
        {
            auto unknown1 = reader.read<uint16_t>();
            auto unknown2 = reader.read<uint32_t>();
            this->message = reader.read<std::string, uint16_t>();
            break;
        }

        case fb::game::dialog::interaction::INPUT_EX:
        {
            reader.read(nullptr, 0x07); // 7바이트 무시
            this->action = reader.read<uint8_t>();
            if (this->action == 0x02) // OK button
            {
                auto unknown1 = reader.read<uint8_t>();
                this->message = reader.read<std::string, uint8_t>();
            }
            break;
        }

        case fb::game::dialog::interaction::MENU:
        {
            auto unknown = reader.read<uint32_t>();
            this->index  = reader.read<uint16_t>();
            break;
        }

        case fb::game::dialog::interaction::ITEM:
        {
            auto unknown  = reader.read<uint32_t>();
            this->pursuit = reader.read<uint16_t>();
            this->name    = reader.read<std::string, uint8_t>();
            break;
        }

        case fb::game::dialog::interaction::SLOT:
        {
            auto unknown  = reader.read<uint32_t>();
            this->pursuit = reader.read<uint16_t>();
            this->index   = reader.read<uint8_t>();
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
    void deserialize(fb::stream_reader<big_endian>& reader)
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
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->name    = reader.read<std::string, uint8_t>();
        this->message = reader.read<std::string, uint8_t>();
    }
};

}}}} // namespace fb::protocol::game::request

#endif // !__PROTOCOL_REQUEST_GAME_H__