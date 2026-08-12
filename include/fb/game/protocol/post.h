#ifndef __PROTOCOL_GAME_POST_H__
#define __PROTOCOL_GAME_POST_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class post : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x34;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
    uint8_t slot = 0;

public:
    post() = default;

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif