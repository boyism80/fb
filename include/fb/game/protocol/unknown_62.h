#ifndef __PROTOCOL_GAME_UNKNOWN_62_H__
#define __PROTOCOL_GAME_UNKNOWN_62_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class unknown_62 : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x62;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifndef BOT
    const uint8_t     type;
    const std::string url;
    const std::string key;
    const std::string cookie;
#else
    uint8_t     type = 0;
    std::string url;
    std::string key;
    std::string cookie;
#endif

public:
#ifdef BOT
    unknown_62() = default;
#else
    unknown_62(uint8_t type = 0, std::string url = {}, std::string key = {}, std::string cookie = {});
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

} // namespace fb::protocol::game::response

#endif
