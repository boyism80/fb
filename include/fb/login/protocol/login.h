#ifndef __PROTOCOL_LOGIN_LOGIN_H__
#define __PROTOCOL_LOGIN_LOGIN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::login::request {

class login : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x03;

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
    login(const std::string& id, const std::string& pw) :
        id(id),
        pw(pw)
    { }
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