#ifndef __PROTOCOL_GAME_DIALOG_SPELL_H__
#define __PROTOCOL_GAME_DIALOG_SPELL_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

class dialog_spell : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::SPELL;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr appearance;
    const std::string    message;
    const uint16_t       pursuit;
    const uint32_t       oid;
#else
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    uint16_t    pursuit = 0;
#endif

public:
#ifndef BOT
    dialog_spell(const fb::model::object& obj,
                 std::string_view         message,
                 uint32_t                 oid     = 0xFFFFFFFD,
                 uint16_t                 pursuit = 0xFFFF);
    dialog_spell(const fb::game::object& object,
                 std::string_view        message,
                 uint32_t                oid     = 0xFFFFFFFD,
                 uint16_t                pursuit = 0xFFFF);
#else
    dialog_spell() = default;
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
