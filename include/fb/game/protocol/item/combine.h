#ifndef __PROTOCOL_GAME_MIX_H__
#define __PROTOCOL_GAME_MIX_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_combine : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x6B;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    std::vector<uint8_t> indices;
#else
    const std::vector<uint8_t> indices;
#endif

public:
#ifndef BOT
    item_combine() = default;
#else
    item_combine(const std::vector<uint8_t>& indices) :
        indices(indices)
    { }
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