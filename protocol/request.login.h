#ifndef __PROTOCOL_REQUEST_LOGIN_H__
#define __PROTOCOL_REQUEST_LOGIN_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace request { namespace login {

class login : public fb::protocol::base::request
{
public:
    std::string             id;
    std::string             pw;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->id = in_stream.readstr_u8();
        this->pw = in_stream.readstr_u8();
    }
};


class agreement : public fb::protocol::base::request
{
public:
    uint8_t                 enc_type;
    uint8_t                 enc_key_size;
    uint8_t                 enc_key[0x09];

public:
    void deserialize(fb::istream& in_stream)
    {
        this->enc_type = in_stream.read_u8();
        this->enc_key_size = in_stream.read_u8();
        in_stream.read(this->enc_key, this->enc_key_size);
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_LOGIN_H__