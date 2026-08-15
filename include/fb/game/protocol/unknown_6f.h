#ifndef __PROTOCOL_GAME_UNKNOWN_6F_H__
#define __PROTOCOL_GAME_UNKNOWN_6F_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <utility>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class unknown_6f : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    uint8_t  subtype = 1;
    uint16_t count   = 0;
#else
    const uint8_t  subtype;
    const uint16_t count;
#endif

public:
#ifdef BOT
    unknown_6f() = default;
#else
    unknown_6f(uint8_t subtype, uint16_t count);
#endif

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

template <>
class unknown_6f<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6F;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifdef BOT
    uint8_t                                       subtype = 1;
    uint16_t                                      count   = 0;
    std::string                                   name;
    uint32_t                                      value = 0;
    std::string                                   body;
    std::vector<std::pair<std::string, uint32_t>> names;
#else
    const uint8_t                                       subtype;
    const uint16_t                                      count;
    const std::string                                   name;
    const uint32_t                                      value;
    const std::string                                   body;
    const std::vector<std::pair<std::string, uint32_t>> names;
#endif

public:
#ifdef BOT
    unknown_6f() = default;
#else
    unknown_6f(uint8_t                                       subtype,
               uint16_t                                      count,
               std::string                                   name  = {},
               uint32_t                                      value = 0,
               std::string                                   body  = {},
               std::vector<std::pair<std::string, uint32_t>> names = {});
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
