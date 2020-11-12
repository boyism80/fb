#ifndef __PROTOCOL_REQUEST_GAME_ITEM_H__
#define __PROTOCOL_REQUEST_GAME_ITEM_H__

#include "protocol/protocol.h"
#include "model/item/item.h"

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request { namespace item {

class active : public fb::protocol::base::header
{
public:
    uint8_t                 index;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8() - 1;
    }
};


class inactive : public fb::protocol::base::header
{
public:
    equipment::slot         slot;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->slot = equipment::slot(in_stream.read_u8());
    }
};


class drop : public fb::protocol::base::header
{
public:
    uint8_t                 index;
    bool                    all;

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