#ifndef __PROTOCOL_GAME_UPLOAD_H__
#define __PROTOCOL_GAME_UPLOAD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class trade_upload : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x42;

public:
#ifndef BOT
    const uint8_t         index;
    const fb::game::item& item;
    const bool            mine;
#else
    uint8_t     index;
    uint16_t    look;
    uint8_t     color;
    std::string name;
    bool        mine;
#endif

public:
#ifndef BOT
    trade_upload(uint8_t index, const fb::game::item& item, bool mine);
#else
    trade_upload() = default;
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