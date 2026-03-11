#ifndef __PROTOCOL_LOGIN_CREATE_H__
#define __PROTOCOL_LOGIN_CREATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

class create : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x02;

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
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::login::request

#endif