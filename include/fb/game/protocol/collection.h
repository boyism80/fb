#ifndef __PROTOCOL_GAME_COLLECTION_H__
#define __PROTOCOL_GAME_COLLECTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

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

#endif
