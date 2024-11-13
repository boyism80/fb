#ifndef __PROTOCOL_REQUEST_GAME_MAP_H__
#define __PROTOCOL_REQUEST_GAME_MAP_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace game { namespace request { namespace map {

class update : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x05;
    
public:
    point16_t position;
    size8_t   size;
    uint16_t  crc;

public:
    update() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->position.x  = reader.read<uint16_t>();
        this->position.y  = reader.read<uint16_t>();
        this->size.width  = reader.read<uint8_t>();
        this->size.height = reader.read<uint8_t>();
        this->crc         = reader.read<uint16_t>();
    }
};

class world : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x3F;
    
public:
    uint16_t value;
    uint16_t before;
    uint16_t after;

public:
    world() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader)
    {
        this->value  = reader.read<uint16_t>();
        this->before = reader.read<uint16_t>();
        this->after  = reader.read<uint16_t>();
    }
};

}}}}} // namespace fb::protocol::game::request::map

#endif // !__PROTOCOL_REQUEST_GAME_MAP_H__