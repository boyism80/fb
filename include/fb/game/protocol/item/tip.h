#ifndef __PROTOCOL_GAME_TIP_H__
#define __PROTOCOL_GAME_TIP_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class item_tip : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x59;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifndef BOT
    const uint16_t    position;
    const std::string message;
#else
    uint16_t    position;
    std::string message;
#endif

public:
#ifndef BOT
    item_tip(uint16_t position, std::string_view message);
#else
    item_tip() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

using item_tip_v550 = item_tip<CLIENT_VERSION::v550>;
using item_tip_v565 = item_tip<CLIENT_VERSION::v565>;
using item_tip_v651 = item_tip<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
