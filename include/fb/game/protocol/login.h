#ifndef __GAME_PROTOCOL_LOGIN_H__
#define __GAME_PROTOCOL_LOGIN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/client_version.h>

namespace fb::protocol::game::request {

/**
 * C2S game login (opcode 0x10). Echoed transfer blob:
 *   enc | key | from | client_version u16 | uid | name | transfer?
 * Packed 651 + NEW UI appends CLIENT_UI_MODE. Packed 651 + OLD UI does not.
 * Bootstrap deserializes as login<v550>; the packed u16 and remaining bytes
 * select whether the UI byte is present.
 */
template <CLIENT_VERSION V>
class login : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode  = 0x10;
    static constexpr bool    decrypt = false;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    struct transfer_param
    {
    public:
        uint16_t                   map;
        fb::model::point<uint16_t> position;
    };

public:
#ifndef BOT
    fb::protocol::internal::Service from;
#else
    uint8_t from;
#endif
    uint8_t                       enc_type;
    uint8_t                       key_size;
    uint8_t                       enc_key[0x09];
    uint32_t                      id;
    std::string                   name;
    CLIENT_VERSION                client_version = CLIENT_VERSION::v550;
    CLIENT_UI_MODE                ui_mode        = CLIENT_UI_MODE::OLD;
    std::optional<transfer_param> transfer;

public:
#ifndef BOT
    login() = default;
#else
    login(const fb::stream& params);
#endif

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
    void deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif
