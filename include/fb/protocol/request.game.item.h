#ifndef __PROTOCOL_REQUEST_GAME_ITEM_H__
#define __PROTOCOL_REQUEST_GAME_ITEM_H__

#include <fb/protocol/protocol.h>
#include <item.h>
#include <model.h>

using namespace fb::model::enum_value;

using namespace fb::game;

namespace fb { namespace protocol { namespace game { namespace request { namespace item {

class active : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1C;
    
public:
    uint8_t index;

public:
    active() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8() - 1;
    }
};

class inactive : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x1F;
    
public:
    EQUIPMENT_PARTS parts;

public:
    inactive() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->parts = EQUIPMENT_PARTS(in_stream.read_u8());
    }
};

class drop : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x08;
    
public:
    uint8_t index;
    bool    all;

public:
    drop() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->index = in_stream.read_u8() - 1;
        this->all   = bool(in_stream.read_u8());
    }
};

class drop_cash : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x24;
    
public:
    uint32_t chunk;

public:
    drop_cash() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->chunk = in_stream.read_u32();
    }
};

class mix : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x6B;
    
public:
    std::vector<uint8_t> indices;

public:
    mix() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        auto count = in_stream.read_u8();
        for (int i = 0; i < count; i++)
            this->indices.push_back(in_stream.read_u8());
    }
};

class throws : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x17;
    
public:
    bool    all;
    uint8_t index;

public:
    throws() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->all   = in_stream.read_u8();
        this->index = in_stream.read_u8() - 1;
    }
};

class info : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x66;
    
public:
    uint16_t position;
    uint8_t  slot;

public:
    info() = default;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->position = in_stream.read_u16();
        auto unknown1  = in_stream.read_u8();
        auto unknown2  = in_stream.read_u8();
        auto unknown3  = in_stream.read_u8();
        this->slot     = in_stream.read_u8() - 1;
    }
};

}}}}} // namespace fb::protocol::game::request::item

#endif // !__PROTOCOL_REQUEST_GAME_ITEM_H__