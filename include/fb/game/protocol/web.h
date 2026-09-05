#ifndef __PROTOCOL_GAME_WEB_H__
#define __PROTOCOL_GAME_WEB_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class web : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x66;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint8_t     type;
    const std::string address;
    const std::string message;
#else
    uint8_t     type;
    std::string address;
    std::string message;
#endif

public:
#ifndef BOT
    web(uint8_t type, std::string address, std::string message);
#else
    web() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using web_v550 = web<CLIENT_VERSION::v550>;
using web_v565 = web<CLIENT_VERSION::v565>;
using web_v651 = web<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
