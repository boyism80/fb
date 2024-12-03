#ifndef __PROTOCOL_REQUEST_LOGIN_H__
#define __PROTOCOL_REQUEST_LOGIN_H__

#include <fb/protocol/protocol.h>

namespace fb::protocol::login::request {

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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<std::string, uint8_t>(this->id);
        writer.write<std::string, uint8_t>(this->pw);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->id = reader.read<std::string, uint8_t>();
        this->pw = reader.read<std::string, uint8_t>();
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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
    	co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->enc_type);
        writer.write<uint8_t>(this->enc_key_size);
        writer.write((const void*)this->enc_key, this->enc_key_size);
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        this->enc_type     = reader.read<uint8_t>();
        this->enc_key_size = reader.read<uint8_t>();
        reader.read(this->enc_key, this->enc_key_size);
    }
#endif
};

} // namespace fb::protocol::login::request

#endif // !__PROTOCOL_REQUEST_LOGIN_H__