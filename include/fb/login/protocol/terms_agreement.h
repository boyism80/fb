#ifndef __PROTOCOL_LOGIN_TERMS_AGREEMENT_H__
#define __PROTOCOL_LOGIN_TERMS_AGREEMENT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

class agreement : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x10;

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
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

namespace fb::protocol::login::response {

class terms_agreement : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x60;

public:
#ifdef BOT
    std::string contents;
#else
    const std::string contents;
#endif

public:
#ifdef BOT
    terms_agreement() = default;
#else
    terms_agreement(std::string_view contents);
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::response

#endif
