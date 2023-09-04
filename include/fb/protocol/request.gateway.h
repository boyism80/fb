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
    assert_version() : fb::protocol::base::header(0x00)
    { }
#ifdef BOT
    assert_version(uint16_t version, uint8_t national_key) : fb::protocol::base::header(0x00),
        version(version), national_key(national_key)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u16(this->version)
                  .write_u8(this->national_key);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->version = in_stream.read_u16();
        this->national_key = in_stream.read_u8();
    }
#endif
};


class entry_list : public fb::protocol::base::header
{
public:
    uint8_t                 action;
    uint8_t                 index;

public:
#ifndef BOT
    entry_list() : fb::protocol::base::header(0x57)
    { }
#else
    entry_list(uint8_t action, uint8_t index) : fb::protocol::base::header(0x57),
        action(action), index(index)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->action);

        if(this->action == 0x00)
            out_stream.write_u8(this->index);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->action = in_stream.read_u8();
        if(action == 0x00)
            this->index = in_stream.read_u8();
    }
#endif
};

} } } }

#endif // !__PROTOCOL_REQUEST_GATEWAY_H__