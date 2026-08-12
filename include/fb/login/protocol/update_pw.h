#ifndef __PROTOCOL_LOGIN_CHANGE_PW_H__
#define __PROTOCOL_LOGIN_CHANGE_PW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

/**
 * C2S change password (opcode 0x26).
 * Primary layout (v550): name | pw | new_pw (all u8-prefixed) + birthday u32.
 * No version delta known for v565/v651.
 */
template <CLIENT_VERSION V>
class update_pw : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x26;
    FB_PROTOCOL_VERSION_TAGS(V);

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
    update_pw() = default;
#else
    update_pw(std::string_view name, std::string_view pw, std::string_view new_pw, uint32_t birthday);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif