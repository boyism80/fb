#ifndef __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__
#define __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__

#include "protocol/protocol.h"

namespace fb { namespace protocol { namespace login { namespace request { namespace account {

class create : public fb::protocol::base::header
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


class complete : public fb::protocol::base::header
{
public:
    uint8_t                 hair;
    uint8_t                 sex;
    uint8_t                 nation;
    uint8_t                 creature;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->hair = in_stream.read_u8();
        this->sex = in_stream.read_u8();
        this->nation = in_stream.read_u8();
        this->creature = in_stream.read_u8();
    }
};


class change_pw : public fb::protocol::base::header
{
public:
    std::string             name;
    std::string             pw;
    std::string             new_pw;
    uint32_t                birthday;

public:
    void deserialize(fb::istream& in_stream)
    {
        this->name = in_stream.readstr_u8();
        this->pw = in_stream.readstr_u8();
        this->new_pw = in_stream.readstr_u8();
        this->birthday = in_stream.read_u32(buffer::endian::BIG);
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__