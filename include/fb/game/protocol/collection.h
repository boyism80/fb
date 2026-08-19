#ifndef __PROTOCOL_GAME_COLLECTION_H__
#define __PROTOCOL_GAME_COLLECTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <vector>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class collection : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x27;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    const uint8_t slot;
#else
    uint8_t slot = 0;
#endif

public:
#ifdef BOT
    explicit collection(uint8_t slot);
#else
    collection() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class collection<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x27;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    uint8_t action = 0;
    uint8_t slot   = 0;
#else
    const uint8_t action = 0;
    const uint8_t slot   = 0;
#endif

public:
#ifndef BOT
    collection() = default;
#else
    collection(uint8_t action, uint8_t slot = 0);
#endif

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

struct collection_entry
{
    uint8_t group_id       = 0;
    uint8_t unlocked_count = 0;
};

inline std::vector<collection_entry> collection_list_groups()
{
    auto entries = std::vector<collection_entry>{};
    entries.reserve(7);
    for (uint8_t g = 0; g < 7; ++g)
        entries.push_back({g, 0});
    return entries;
}

enum class COLLECTION_TYPE : uint8_t
{
    LIST   = 0,
    DIALOG = 1,
    FLAG   = 2,
};

template <CLIENT_VERSION V>
class collection : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x12;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint32_t oid;
    const uint8_t  slot;
    const uint8_t  flag;
#else
    uint32_t oid  = 0;
    uint8_t  slot = 0;
    uint8_t  flag = 0;
#endif

public:
#ifndef BOT
    collection(uint32_t oid, uint8_t slot, uint8_t flag);
#else
    collection() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class collection<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x12;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    const COLLECTION_TYPE              type;
    const std::vector<collection_entry> entries;
    const uint8_t                      group_id;
    const std::vector<uint8_t>         bitmask;
    const uint8_t                      slot;
    const bool                         onoff;
#else
    COLLECTION_TYPE              type = COLLECTION_TYPE::FLAG;
    std::vector<collection_entry> entries;
    uint8_t                      group_id = 0;
    std::vector<uint8_t>         bitmask;
    uint8_t                      slot  = 0;
    bool                         onoff = false;
#endif

public:
#ifndef BOT
    collection(uint32_t oid, uint8_t slot, uint8_t flag);
    collection(uint8_t group_id, uint8_t slot, bool onoff);
    explicit collection(std::vector<collection_entry> entries);
    collection(uint8_t group_id, std::vector<uint8_t> bitmask);
#else
    collection() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
