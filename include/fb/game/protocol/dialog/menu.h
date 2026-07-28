#ifndef __PROTOCOL_GAME_MENU_H__
#define __PROTOCOL_GAME_MENU_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog_menu : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::MENU;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
#ifndef BOT
    const appearance_ptr           appearance;
    const std::vector<std::string> menus;
    const std::string              message;
    const uint32_t                 oid;
#else
    uint16_t                 look;
    uint8_t                  color;
    std::string              message;
    uint8_t                  type_echo;
    uint32_t                 oid;
    std::vector<std::string> menus;
#endif

public:
#ifndef BOT
    dialog_menu(const fb::model::object&        obj,
                const std::vector<std::string>& menus,
                std::string_view                message,
                uint32_t                        oid = 0xFFFFFFFD);
    dialog_menu(const fb::game::object&         object,
                const std::vector<std::string>& menus,
                std::string_view                message,
                uint32_t                        oid = 0xFFFFFFFD);
#else
    dialog_menu() = default;
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
