#ifndef __PROTOCOL_LOGIN_CHANGE_PW_H__
#define __PROTOCOL_LOGIN_CHANGE_PW_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::login::request {

class update_pw : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x26;

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
    update_pw(const std::string& name, const std::string& pw, const std::string& new_pw, uint32_t birthday);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};


}

#endif