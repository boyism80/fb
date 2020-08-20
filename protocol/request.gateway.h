#ifndef __PROTOCOL_REQUEST_GATEWAY_H__
#define __PROTOCOL_REQUEST_GATEWAY_H__

#include <protocol/protocol.h>

namespace fb { namespace protocol { namespace request { namespace gateway {

class assert_version : public fb::protocol::base::request
{
public:
    uint16_t                version;
    uint8_t                 national_key;
public:
    void deserialize(fb::istream& in_stream)
    {
        this->version = in_stream.read_u16();
        this->national_key = in_stream.read_u8();
    }
};


class entry_list : public fb::protocol::base::request
{
public:
    uint8_t                 action;
    uint8_t                 index;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        if(action == 0x00)
            this->index = in_stream.read_u8();
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_GATEWAY_H__