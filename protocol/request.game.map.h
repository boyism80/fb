#ifndef __PROTOCOL_REQUEST_GAME_MAP_H__
#define __PROTOCOL_REQUEST_GAME_MAP_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace request { namespace game { namespace map {

class update : public fb::protocol::base::request
{
public:
    point16_t               position;
    size8_t                 size;
    uint16_t                crc;

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

} } } } }

#endif // !__PROTOCOL_REQUEST_GAME_MAP_H__