#ifndef __PROTOCOL_LOGIN_AGREEMENT_H__
#define __PROTOCOL_LOGIN_AGREEMENT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::login::request {

class agreement : public fb::protocol::header
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
    agreement(uint8_t type, uint8_t ksize, const uint8_t* key);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

namespace fb::protocol::login::response {

class agreement : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x60;

public:
#ifdef BOT
    std::string contents;
#else
    const std::string contents;
#endif

public:
#ifdef BOT
    agreement() = default;
#else
    agreement(const std::string& contents);
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::response

#endif