#ifndef __PROTOCOL_GAME_GROUP_H__
#define __PROTOCOL_GAME_GROUP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class group : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2E;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    std::string name;
#else
    const std::string name;
#endif

public:
#ifndef BOT
    group() = default;
#else
    group(std::string_view name);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif