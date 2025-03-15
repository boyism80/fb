#ifndef __PROTOCOL_GAME_MENU_H__
#define __PROTOCOL_GAME_MENU_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class dialog_menu : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x2F;

public:
#ifndef BOT
    const fb::model::object&            obj;
    const std::vector<std::string>      menus;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;
#else

#endif

public:
#ifndef BOT
    dialog_menu(const fb::model::object&        obj,
                const std::vector<std::string>& menus,
                const std::string&              message,
                fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::MENU);
#else
    dialog_menu() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif