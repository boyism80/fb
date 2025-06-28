#ifndef __PROTOCOL_GAME_LIST_H__
#define __PROTOCOL_GAME_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class dialog_list : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x30;

public:
#ifndef BOT
    const fb::model::object&            object;
    const std::vector<std::string>      lists;
    const std::string                   message;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;
    const uint32_t                      sequence;
#else
    uint16_t                 look;
    uint8_t                  color;
    std::string              message;
    bool                     button_prev;
    uint8_t                  interaction;
    uint32_t                 sequence;
    std::vector<std::string> lists;
#endif

public:
#ifndef BOT
    dialog_list(const fb::model::object&        object,
                const std::vector<std::string>& list,
                const std::string&              message,
                bool                            button_prev,
                uint32_t                        sequence    = 0xFFFFFFFD,
                fb::game::dialog::interaction   interaction = fb::game::dialog::interaction::LIST);
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