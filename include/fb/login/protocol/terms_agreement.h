#ifndef __PROTOCOL_LOGIN_TERMS_AGREEMENT_H__
#define __PROTOCOL_LOGIN_TERMS_AGREEMENT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

/**
 * C2S agreement (opcode 0x10).
 * Stock clients echo the full gateway->login transfer parameter blob here:
 *   enc_type u8 | key_size u8 | iv[key_size] | from u8 | client_version u16
 */
class agreement : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x10;

public:
#ifdef BOT
    const uint8_t        enc_type;
    const uint8_t        enc_key_size;
    const uint8_t        from;
    const CLIENT_VERSION client_version;
#else
    uint8_t        enc_type;
    uint8_t        enc_key_size;
    uint8_t        from           = 0;
    CLIENT_VERSION client_version = CLIENT_VERSION::v550;
#endif
    uint8_t enc_key[0x09];

public:
#ifndef BOT
    agreement() = default;
#else
    agreement(uint8_t type, uint8_t ksize, const uint8_t* key, uint8_t from, CLIENT_VERSION client_version);
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
