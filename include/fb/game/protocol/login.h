#ifndef __GAME_PROTOCOL_LOGIN_H__
#define __GAME_PROTOCOL_LOGIN_H__

#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <fb/protocol/header.h>
#include <fb/transfer_ticket.h>
#include <macro.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace fb::protocol::game::request {

enum class TRANSFER_PARAM : uint8_t
{
    NONE    = 0x00,
    MAP     = 0x01,
    MATCH   = 0x02,
    UI_MODE = 0x04,
};

/**
 * C2S game login (opcode 0x10). Echoed transfer blob:
 *   enc | key | from | client_version u16 | uid | name | flags u8 | sections | ticket
 * flags: MAP (world/map/xy), MATCH (match_id/match_type/team), UI_MODE (CLIENT_UI_MODE).
 * ticket: expire u32 | nonce u64 | tag (see fb/transfer_ticket.h). The client echoes the blob verbatim.
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
        uint32_t                   world = 0;
        uint16_t                   map;
        fb::model::point<uint16_t> position;
    };

    struct match_param
    {
    public:
        std::string id;
        uint32_t    type = 0;
        uint32_t    team = 0;
    };

    struct ticket_param
    {
    public:
        uint32_t                      expire = 0;
        uint64_t                      nonce  = 0;
        fb::transfer_ticket::tag_type tag    = {};
        std::vector<uint8_t>          signed_bytes;
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
    std::optional<match_param>    match;
    std::optional<ticket_param>   ticket;

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
