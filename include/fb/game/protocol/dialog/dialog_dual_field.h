#ifndef __PROTOCOL_GAME_DIALOG_DUAL_FIELD_H__
#define __PROTOCOL_GAME_DIALOG_DUAL_FIELD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

#include <utility>

namespace fb::protocol::game::response {

class dialog_dual_field : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::DUAL_FIELD;
#ifndef BOT
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
    using pair_list      = std::vector<std::pair<std::string, std::string>>;
#endif

public:
#ifndef BOT
    const appearance_ptr appearance;
    const pair_list      pairs;
    const std::string    message;
    const uint16_t       pursuit;
    const uint32_t       oid;
#else
    uint8_t                                          type_echo = 0;
    uint32_t                                         oid       = 0;
    std::string                                      message;
    uint16_t                                         pursuit = 0;
    std::vector<std::pair<std::string, std::string>> pairs;
#endif

public:
#ifndef BOT
    dialog_dual_field(const fb::model::object& obj,
                      const pair_list&         pairs,
                      std::string_view         message,
                      uint32_t                 oid     = 0xFFFFFFFD,
                      uint16_t                 pursuit = 0xFFFF);
    dialog_dual_field(const fb::game::object& object,
                      const pair_list&        pairs,
                      std::string_view        message,
                      uint32_t                oid     = 0xFFFFFFFD,
                      uint16_t                pursuit = 0xFFFF);
#else
    dialog_dual_field() = default;
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
