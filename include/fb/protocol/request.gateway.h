#ifndef __PROTOCOL_REQUEST_GATEWAY_H__
#define __PROTOCOL_REQUEST_GATEWAY_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace gateway { namespace request {

class assert_version : public fb::protocol::base::header
{
public:
    uint16_t                version;
    uint8_t                 national_key;

public:
#ifdef BOT
    assert_version()
    { }

    assert_version(uint16_t version, uint8_t national_key) : version(version), national_key(national_key)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x00)
            .write_u16(this->version)
            .write_u8(this->national_key);
    }
#endif
    void deserialize(fb::istream& in_stream)
    {
        this->version = in_stream.read_u16();
        this->national_key = in_stream.read_u8();
    }
};


class entry_list : public fb::protocol::base::header
{
public:
    uint8_t                 action;
    uint8_t                 index;

#ifdef BOT
public:
    entry_list()
    { }

    entry_list(uint8_t action, uint8_t index) : action(action), index(index)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(0x57)
            .write_u8(this->action);

        if(this->action == 0x00)
            out_stream.write_u8(this->index);
    }
#endif

    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        if(action == 0x00)
            this->index = in_stream.read_u8();
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_GATEWAY_H__