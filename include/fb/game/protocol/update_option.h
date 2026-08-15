#ifndef __PROTOCOL_GAME_CHANGE_OPTION_H__
#define __PROTOCOL_GAME_CHANGE_OPTION_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <vector>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class update_option : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1B;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    OPTION option;
    bool   ride = false;
#else
    const OPTION option;
    const bool   ride = false;
#endif

public:
#ifndef BOT
    update_option() = default;
#else
    update_option(OPTION option, bool ride);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <>
class update_option<CLIENT_VERSION::v651> : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1B;
    FB_PROTOCOL_VERSION_TAGS(CLIENT_VERSION::v651);

public:
#ifndef BOT
    std::vector<OPTION> options;
    bool                ride = false;
#else
    const std::vector<OPTION> options;
    const bool                ride = false;
#endif

public:
#ifndef BOT
    update_option() = default;
#else
    update_option(OPTION option, bool ride);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using update_option_v550 = update_option<CLIENT_VERSION::v550>;
using update_option_v565 = update_option<CLIENT_VERSION::v565>;
using update_option_v651 = update_option<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::request

#endif
