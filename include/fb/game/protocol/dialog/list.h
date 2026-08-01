#ifndef __PROTOCOL_GAME_LIST_H__
#define __PROTOCOL_GAME_LIST_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <optional>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class dialog_list : public fb::protocol::header
{
public:
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
#endif

public:
    static constexpr uint8_t opcode = 0x30;

public:
#ifndef BOT
    const appearance_ptr             appearance;
    const std::vector<std::string>   lists;
    const std::optional<std::string> message; // nullopt → subtype 3 (no text)
    const bool                       button_prev;
    const uint32_t                   oid;
#else
    uint16_t                   look;
    uint8_t                    color;
    std::optional<std::string> message;
    bool                       button_prev;
    uint8_t                    type_echo;
    uint32_t                   oid;
    std::vector<std::string>   lists;
#endif

public:
#ifndef BOT
    dialog_list(const fb::game::object&         object,
                const std::vector<std::string>& list,
                std::optional<std::string>      message,
                bool                            button_prev,
                uint32_t                        oid = 0xFFFFFFFD);
    dialog_list(const fb::model::object&        object,
                const std::vector<std::string>& list,
                std::optional<std::string>      message,
                bool                            button_prev,
                uint32_t                        oid = 0xFFFFFFFD);
    dialog_list(appearance_ptr&&                appearance,
                const std::vector<std::string>& list,
                std::optional<std::string>      message,
                bool                            button_prev,
                uint32_t                        oid = 0xFFFFFFFD);
#else
    dialog_list() = default;
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
