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
 * Packed 651 always appends CLIENT_UI_MODE (g_ui_mode_new). 550/565 stop after
 * client_version. Bootstrap deserializes as agreement<v550>; the packed u16
 * selects whether the UI byte is present.
 */
template <CLIENT_VERSION V>
class agreement : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x10;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    const uint8_t        enc_type;
    const uint8_t        enc_key_size;
    const uint8_t        from;
    const CLIENT_VERSION client_version;
    const CLIENT_UI_MODE ui_mode;
#else
    uint8_t        enc_type;
    uint8_t        enc_key_size;
    uint8_t        from           = 0;
    CLIENT_VERSION client_version = CLIENT_VERSION::v550;
    CLIENT_UI_MODE ui_mode        = CLIENT_UI_MODE::OLD;
#endif
    uint8_t enc_key[0x09];

public:
#ifndef BOT
    agreement() = default;
#else
    agreement(uint8_t        type,
              uint8_t        ksize,
              const uint8_t* key,
              uint8_t        from,
              CLIENT_VERSION client_version,
              CLIENT_UI_MODE ui_mode = CLIENT_UI_MODE::OLD);
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
