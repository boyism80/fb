#ifndef __PROTOCOL_REQUEST_LOGIN_H__
#define __PROTOCOL_REQUEST_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace request {

class login : public fb::protocol::base::header
{
public:
    std::string             id;
    std::string             pw;

public:
    login() : fb::protocol::base::header(0x03)
    { }

public:
    void deserialize(fb::istream& in_stream)
    {
        this->id = in_stream.readstr_u8();
        this->pw = in_stream.readstr_u8();
    }
};


class agreement : public fb::protocol::base::header
{
public:
    uint8_t                 enc_type;
    uint8_t                 enc_key_size;
    uint8_t                 enc_key[0x09];

public:
    agreement() : fb::protocol::base::header(0x10)
    { }
#ifdef BOT
    agreement(uint8_t type, uint8_t ksize, const uint8_t* key) : agreement()
    {
        this->enc_type = type;
        this->enc_key_size = ksize;
        memcpy(this->enc_key, key, ksize);
    }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->enc_type)
                  .write_u8(this->enc_key_size)
                  .write((const void*)this->enc_key, this->enc_key_size);
    }
#endif
    void deserialize(fb::istream& in_stream)
    {
        this->enc_type = in_stream.read_u8();
        this->enc_key_size = in_stream.read_u8();
        in_stream.read(this->enc_key, this->enc_key_size);
    }
};

} } } }

#endif // !__PROTOCOL_REQUEST_LOGIN_H__