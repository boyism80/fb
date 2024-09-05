#ifndef __PROTOCOL_REQUEST_GAME_MAP_H__
#define __PROTOCOL_REQUEST_GAME_MAP_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace game { namespace request { namespace map {

class update : public fb::protocol::base::header
{
public:
    point16_t                   position;
    size8_t                     size;
    uint16_t                    crc;

public:
    update() : fb::protocol::base::header(0x05)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->position.x = in_stream.read_u16();
        this->position.y = in_stream.read_u16();
        this->size.width = in_stream.read_u8();
        this->size.height = in_stream.read_u8();
        this->crc = in_stream.read_u16();
    }
};

class world : public fb::protocol::base::header
{
public:
    uint16_t                value;
    uint16_t                before;
    uint16_t                after;

public:
    world() : fb::protocol::base::header(0x3F)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->value = in_stream.read_u16();
        this->before = in_stream.read_u16();
        this->after = in_stream.read_u16();
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_GAME_MAP_H__