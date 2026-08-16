#ifndef __PROTOCOL_GAME_WEB_MAP_H__
#define __PROTOCOL_GAME_WEB_MAP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <vector>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class web_map : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x7C;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
    web_map() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

struct web_map_entry
{
    uint16_t    x = 0;
    uint16_t    y = 0;
    std::string name;
};

template <CLIENT_VERSION V>
class web_map : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x70;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v651);

public:
#ifdef BOT
    std::vector<web_map_entry> entries;
#else
    const std::vector<web_map_entry> entries;
#endif

public:
#ifdef BOT
    web_map() = default;
#else
    explicit web_map(const std::vector<web_map_entry>& entries);
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
