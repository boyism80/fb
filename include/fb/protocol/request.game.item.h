#ifndef __PROTOCOL_REQUEST_GAME_ITEM_H__
#define __PROTOCOL_REQUEST_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <fb/game/item.h>
#include <fb/game/model.h>

using namespace fb::model::enum_value;

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request { namespace item {

class active : public fb::protocol::base::header
{
public:
    uint8_t                 index;

public:
    active() : fb::protocol::base::header(0x1C)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8() - 1;
    }
};


class inactive : public fb::protocol::base::header
{
public:
    EQUIPMENT_PARTS         parts;

public:
    inactive() : fb::protocol::base::header(0x1F)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->parts = EQUIPMENT_PARTS(in_stream.read_u8());
    }
};


class drop : public fb::protocol::base::header
{
public:
    uint8_t                 index;
    bool                    all;

public:
    drop() : fb::protocol::base::header(0x08)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8() - 1;
        this->all = bool(in_stream.read_u8());
    }
};


class drop_cash : public fb::protocol::base::header
{
public:
    uint32_t                chunk;

public:
    drop_cash() : fb::protocol::base::header(0x24)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->chunk = in_stream.read_u32();
    }
};

class mix : public fb::protocol::base::header
{
public:
    std::vector<uint8_t>    indices;

public:
    mix() : fb::protocol::base::header(0x6B)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        auto count = in_stream.read_u8();
        for(int i = 0; i < count; i++)
            this->indices.push_back(in_stream.read_u8());
    }
};


class throws : public fb::protocol::base::header
{
public:
    bool                    all;
    uint8_t                 index;

public:
    throws() : fb::protocol::base::header(0x17)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->all = in_stream.read_u8();
        this->index = in_stream.read_u8() - 1;
    }
};


class info : public fb::protocol::base::header
{
public:
    uint16_t                position;
    uint8_t                 slot;

public:
    info() : fb::protocol::base::header(0x66)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->position = in_stream.read_u16();
        auto unknown1 = in_stream.read_u8();
        auto unknown2 = in_stream.read_u8();
        auto unknown3 = in_stream.read_u8();
        this->slot = in_stream.read_u8() - 1;
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_GAME_ITEM_H__