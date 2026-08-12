#ifndef __PROTOCOL_GAME_USER_INFO_SUBMIT_H__
#define __PROTOCOL_GAME_USER_INFO_SUBMIT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <array>
#include <string>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

/**
 * C2S 0x53 — USERINFO dialog OK submit (UserInfoDlg_OnOk).
 * Wire: sub (usually 1) + 8 length-prefixed CP949 strings (4 text + 4 combo labels).
 */
template <CLIENT_VERSION V>
class user_info_submit : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x53;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    const uint8_t                    sub;
    const std::array<std::string, 8> strings;
#else
    uint8_t                    sub = 0;
    std::array<std::string, 8> strings;
#endif

public:
#ifdef BOT
    user_info_submit(uint8_t sub, const std::array<std::string, 8>& strings) :
        sub(sub),
        strings(strings)
    { }
#else
    user_info_submit() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
