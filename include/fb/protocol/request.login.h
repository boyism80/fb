#ifndef __PROTOCOL_REQUEST_LOGIN_H__
#define __PROTOCOL_REQUEST_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace request {

class login : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x03;

public:
#ifdef BOT
    const std::string id;
    const std::string pw;
#else
    std::string id;
    std::string pw;
#endif

public:
#ifndef BOT
    login() = default;
#else
    login(const std::string& id, const std::string& pw) :
        id(id),
        pw(pw)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.writestr_u8(this->id).writestr_u8(this->pw);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->id = in_stream.readstr_u8();
        this->pw = in_stream.readstr_u8();
    }
#endif
};

class agreement : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x10;

public:
#ifdef BOT
    const uint8_t enc_type;
    const uint8_t enc_key_size;
#else
    uint8_t enc_type;
    uint8_t enc_key_size;
#endif
    uint8_t enc_key[0x09];

public:
#ifndef BOT
    agreement() = default;
#else
    agreement(uint8_t type, uint8_t ksize, const uint8_t* key) :
        enc_type(type),
        enc_key_size(ksize)
    {
        memcpy(this->enc_key, key, ksize);
    }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        out_stream.write_u8(header);
        out_stream.write_u8(this->enc_type)
            .write_u8(this->enc_key_size)
            .write((const void*)this->enc_key, this->enc_key_size);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->enc_type     = in_stream.read_u8();
        this->enc_key_size = in_stream.read_u8();
        in_stream.read(this->enc_key, this->enc_key_size);
    }
#endif
};

}}}} // namespace fb::protocol::login::request

#endif // !__PROTOCOL_REQUEST_LOGIN_H__