#ifndef __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__
#define __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__

#include <fb/protocol/protocol.h>

namespace fb { namespace protocol { namespace login { namespace request { namespace account {

class create : public fb::protocol::base::header
{
public:
#ifndef BOT
    std::string id;
    std::string pw;
#else
    const std::string id;
    const std::string pw;
#endif

public:
#ifndef BOT
    create() :
        fb::protocol::base::header(0x02)
    { }
#else
    create(const std::string& id, const std::string& pw) :
        fb::protocol::base::header(0x02),
        id(id),
        pw(pw)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
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

class complete : public fb::protocol::base::header
{
public:
#ifndef BOT
    uint8_t hair;
    uint8_t sex;
    uint8_t nation;
    uint8_t creature;
#else
    const uint8_t hair;
    const uint8_t sex;
    const uint8_t nation;
    const uint8_t creature;
#endif

public:
#ifndef BOT
    complete() :
        fb::protocol::base::header(0x04)
    { }
#else
    complete(uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature) :
        fb::protocol::base::header(0x04),
        hair(hair),
        sex(sex),
        nation(nation),
        creature(creature)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8(this->hair).write_u8(this->sex).write_u8(this->nation).write_u8(this->creature);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->hair     = in_stream.read_u8();
        this->sex      = in_stream.read_u8();
        this->nation   = in_stream.read_u8();
        this->creature = in_stream.read_u8();
    }
#endif
};

class change_pw : public fb::protocol::base::header
{
public:
#ifndef BOT
    std::string name;
    std::string pw;
    std::string new_pw;
    uint32_t    birthday;
#else
    const std::string name;
    const std::string pw;
    const std::string new_pw;
    const uint32_t    birthday;
#endif

public:
#ifndef BOT
    change_pw() :
        fb::protocol::base::header(0x26)
    { }
#else
    change_pw(const std::string& name, const std::string& pw, const std::string& new_pw, uint32_t birthday) :
        fb::protocol::base::header(0x26),
        name(name),
        pw(pw),
        new_pw(new_pw),
        birthday(birthday)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.writestr_u8(this->name).writestr_u8(this->pw).writestr_u8(this->new_pw).write_u32(this->birthday, buffer::endian::BIG);
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->name     = in_stream.readstr_u8();
        this->pw       = in_stream.readstr_u8();
        this->new_pw   = in_stream.readstr_u8();
        this->birthday = in_stream.read_u32(buffer::endian::BIG);
    }
#endif
};

}}}}} // namespace fb::protocol::login::request::account

#endif // !__PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__