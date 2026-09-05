#ifndef __PROTOCOL_LOGIN_LOGIN_H__
#define __PROTOCOL_LOGIN_LOGIN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

/**
 * C2S login (opcode 0x03).
 * Primary layout (v550): id (u8-prefixed) + pw (u8-prefixed).
 * No version delta known for v565/v651.
 */
template <CLIENT_VERSION V>
class login : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x03;
    FB_PROTOCOL_VERSION_TAGS(V);

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
    login(std::string_view id, std::string_view pw);
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