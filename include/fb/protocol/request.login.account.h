#ifndef __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__
#define __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace request { namespace account {

class create : public fb::protocol::base::header
{
public:
    std::string             id;
    std::string             pw;

public:
    create() : fb::protocol::base::header(0x02)
    { }
#ifdef BOT
    create(const std::string& id, const std::string& pw) : create()
    { 
        this->id = id;
        this->pw = pw;
    }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.writestr_u8(this->id)
                  .writestr_u8(this->pw);
    }
#endif
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
    complete() : fb::protocol::base::header(0x04)
    { }
#ifdef BOT
    complete(uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature) : fb::protocol::base::header(0x04),
        hair(hair), sex(sex), nation(nation), creature(creature)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->hair)
                  .write_u8(this->sex)
                  .write_u8(this->nation)
                  .write_u8(this->creature);
    }
#endif
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
    change_pw() : fb::protocol::base::header(0x26)
    { }

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