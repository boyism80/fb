#ifndef __PROTOCOL_GAME_ITEM_UPDATE_H__
#define __PROTOCOL_GAME_ITEM_UPDATE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string>
#ifndef BOT
#include <fb/game/character.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class item_update : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0F;

public:
#ifndef BOT
    const uint8_t     index;
    const uint16_t    look;
    const uint8_t     color;
    const std::string name;
    const uint32_t    count;
    const bool        valid;
#else
    uint8_t     index;
    uint16_t    look;
    uint8_t     color;
    std::string name;
    uint32_t    count;
#endif

public:
#ifndef BOT
    item_update(const fb::game::character& me, uint8_t index);
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count);
#else
    item_update() = default;
#endif

private:
#ifndef BOT
    item_update(uint8_t index, uint16_t look, uint8_t color, std::string name, uint32_t count, bool valid);
    static item_update from_inventory(const fb::game::character& me, uint8_t index);
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
