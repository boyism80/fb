#ifndef __PROTOCOL_GAME_UNKNOWN_7B_H__
#define __PROTOCOL_GAME_UNKNOWN_7B_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

/**
 * C2S 0x7B — name miss report for the S2C 0x6F name list, since 5.65.
 * Wire: [0x7B][0x00][name_len u8][name CP949].
 */
template <CLIENT_VERSION V>
class unknown_7b : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x7B;
    FB_PROTOCOL_VERSION_TAGS_SINCE(V, CLIENT_VERSION::v565);

public:
#ifdef BOT
    const std::string name;
#else
    std::string name;
#endif

public:
#ifdef BOT
    explicit unknown_7b(const std::string& name) :
        name(name)
    { }
#else
    unknown_7b() = default;
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
