#ifndef __PROTOCOL_GAME_UNKNOWN_4F_H__
#define __PROTOCOL_GAME_UNKNOWN_4F_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>
#include <vector>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

struct unknown_4f_entry
{
    uint32_t    id    = 0;
    uint32_t    pad   = 0;
    uint16_t    field = 0;
    uint8_t     kind  = 0;
    std::string name;
    uint8_t     percent = 255;
    uint32_t    a       = 0;
    uint32_t    b       = 0;
    uint32_t    c       = 0;
};

template <CLIENT_VERSION V>
class unknown_4f : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4F;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
    unknown_4f() = default;

public:
#ifdef BOT
    void deserialize(fb::stream_reader<big_endian>& reader);
#else
    void serialize(fb::stream_writer<big_endian>& writer) const;
#endif
};

template <>
class unknown_4f<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4F;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    const uint32_t                      token;
    const uint32_t                      body_a;
    const uint8_t                       alloc;
    const uint8_t                       flag;
    const uint32_t                      body_b;
    const std::vector<unknown_4f_entry> entries;
#else
    uint32_t                      token  = 0;
    uint32_t                      body_a = 0;
    uint8_t                       alloc  = 0;
    uint8_t                       flag   = 0;
    uint32_t                      body_b = 0;
    std::vector<unknown_4f_entry> entries;
#endif

public:
#ifdef BOT
    unknown_4f() = default;
#else
    unknown_4f(uint32_t                      token   = 0,
               uint32_t                      body_a  = 0,
               uint8_t                       alloc   = 0,
               uint8_t                       flag    = 0,
               uint32_t                      body_b  = 0,
               std::vector<unknown_4f_entry> entries = {});
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
