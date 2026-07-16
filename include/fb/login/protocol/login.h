#ifndef __PROTOCOL_LOGIN_LOGIN_H__
#define __PROTOCOL_LOGIN_LOGIN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::login::request {

using namespace fb::model::enum_value;

class login : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x03;

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
    login(std::string_view id, std::string_view pw) :
        id(std::string(id)),
        pw(std::string(pw))
    { }
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