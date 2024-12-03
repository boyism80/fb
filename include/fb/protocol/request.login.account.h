#ifndef __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__
#define __PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__

#include <fb/protocol/protocol.h>

namespace fb::protocol::login::request::account {

class create : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x02;

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
    create() = default;
#else
    create(const std::string& id, const std::string& pw) :
        id(id),
        pw(pw)
    { }
#endif

public:
#ifdef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string, uint8_t>(this->id);
        writer.write<std::string, uint8_t>(this->pw);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->id = reader.read<std::string, uint8_t>();
        this->pw = reader.read<std::string, uint8_t>();
    }
#endif
};

class complete : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x04;

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
    complete() = default;
#else
    complete(uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature) :
        hair(hair),
        sex(sex),
        nation(nation),
        creature(creature)
    { }
#endif

public:
#ifdef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->hair);
        writer.write<uint8_t>(this->sex);
        writer.write<uint8_t>(this->nation);
        writer.write<uint8_t>(this->creature);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->hair     = reader.read<uint8_t>();
        this->sex      = reader.read<uint8_t>();
        this->nation   = reader.read<uint8_t>();
        this->creature = reader.read<uint8_t>();
    }
#endif
};

class change_pw : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x26;

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
    change_pw() = default;
#else
    change_pw(const std::string& name, const std::string& pw, const std::string& new_pw, uint32_t birthday) :
        name(name),
        pw(pw),
        new_pw(new_pw),
        birthday(birthday)
    { }
#endif

public:
#ifdef BOT
    async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string, uint8_t>(this->name);
        writer.write<std::string, uint8_t>(this->pw);
        writer.write<std::string, uint8_t>(this->new_pw);
        writer.write<uint32_t>(this->birthday);
    }
#else
    async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->name     = reader.read<std::string, uint8_t>();
        this->pw       = reader.read<std::string, uint8_t>();
        this->new_pw   = reader.read<std::string, uint8_t>();
        this->birthday = reader.read<uint32_t>();
    }
#endif
};

} // namespace fb::protocol::login::request::account

#endif // !__PROTOCOL_REQUEST_LOGIN_ACCOUNT_H__