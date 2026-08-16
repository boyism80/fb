#ifndef __PROTOCOL_LOGIN_CREATE_H__
#define __PROTOCOL_LOGIN_CREATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/protocol/client_version.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

/**
 * C2S create account (opcode 0x02).
 * Primary layout (v550): id (u8-prefixed) + pw (u8-prefixed).
 * v651 also sends a third string8 (and a trailing NUL in send length); those
 * bytes are consumed and discarded.
 */
template <CLIENT_VERSION V>
class create : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x02;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    std::string id;
    std::string pw;
#else
    const std::string id;
    const std::string pw;
#endif

public:
#ifndef BOT
    create() = default;
#else
    create(std::string_view id, std::string_view pw);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <>
void create<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

} // namespace fb::protocol::login::request

#endif