#ifndef __PROTOCOL_LOGIN_CREATE_H__
#define __PROTOCOL_LOGIN_CREATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::login::request {

class create : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x02;

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
    create(const std::string& id, const std::string& pw);
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