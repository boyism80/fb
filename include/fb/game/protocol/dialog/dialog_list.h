#ifndef __PROTOCOL_GAME_LIST_H__
#define __PROTOCOL_GAME_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog_list : public fb::protocol::header
{
public:
#ifndef BOT
    using portrait_ptr = std::unique_ptr<fb::game::portrait>;
#endif

public:
    static constexpr uint8_t header = 0x30;

public:
#ifndef BOT
    const portrait_ptr                  portrait;
    const std::vector<std::string>      lists;
    const std::string                   message;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      oid;
#else
    uint16_t                 look;
    uint8_t                  color;
    std::string              message;
    bool                     button_prev;
    uint8_t                  interaction;
    uint32_t                 oid;
    std::vector<std::string> lists;
#endif

public:
#ifndef BOT
    dialog_list(const fb::game::object&         object,
                const std::vector<std::string>& list,
                std::string_view                  message,
                bool                             button_prev,
                uint32_t                         oid         = 0xFFFFFFFD,
                fb::game::dialog::interaction    interaction = fb::game::dialog::interaction::LIST);
    dialog_list(const fb::model::object&        object,
                const std::vector<std::string>& list,
                std::string_view                  message,
                bool                             button_prev,
                uint32_t                         oid         = 0xFFFFFFFD,
                fb::game::dialog::interaction    interaction = fb::game::dialog::interaction::LIST);
    dialog_list(portrait_ptr&&                  portrait,
                const std::vector<std::string>& list,
                std::string_view                  message,
                bool                             button_prev,
                uint32_t                         oid         = 0xFFFFFFFD,
                fb::game::dialog::interaction    interaction = fb::game::dialog::interaction::LIST);
#else
    dialog_list() = default;
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