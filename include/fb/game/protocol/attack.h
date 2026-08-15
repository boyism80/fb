#ifndef __PROTOCOL_GAME_ATTACK_H__
#define __PROTOCOL_GAME_ATTACK_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <vector>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class attack : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x13;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    attack() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class attack<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x13;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    uint8_t               kind  = 4;
    uint8_t               delay = 0;
    std::vector<uint32_t> oids;
#endif

public:
    attack() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
