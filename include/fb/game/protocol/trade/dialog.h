#ifndef __PROTOCOL_GAME_TRADE_DIALOG_H__
#define __PROTOCOL_GAME_TRADE_DIALOG_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class trade_dialog : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;
    FB_PROTOCOL_VERSION_TAGS(V);

    static constexpr uint8_t SUBTYPE_OPEN = 0x00;

public:
#ifndef BOT
    const fb::game::character& me;
#else
    uint32_t    oid;
    std::string name;
    int16_t     reputation = 0;
#endif

public:
#ifndef BOT
    trade_dialog(const fb::game::character& me);
#else
    trade_dialog() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

#ifndef BOT
template <>
void trade_dialog<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;
#else
template <>
void trade_dialog<CLIENT_VERSION::v651>::deserialize(fb::stream_reader<big_endian>& reader);
#endif

using trade_dialog_v550 = trade_dialog<CLIENT_VERSION::v550>;
using trade_dialog_v565 = trade_dialog<CLIENT_VERSION::v565>;
using trade_dialog_v651 = trade_dialog<CLIENT_VERSION::v651>;

} // namespace fb::protocol::game::response

#endif
