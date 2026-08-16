#ifndef __PROTOCOL_GAME_SELF_INFO_H__
#define __PROTOCOL_GAME_SELF_INFO_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class self_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2D;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    self_info() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class self_info<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2D;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    bool party = false;
#else
    const bool party = false;
#endif

public:
#ifndef BOT
    self_info() = default;
#else
    self_info(bool party = false);
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
