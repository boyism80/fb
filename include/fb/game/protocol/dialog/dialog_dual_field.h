#ifndef __PROTOCOL_GAME_DIALOG_DUAL_FIELD_H__
#define __PROTOCOL_GAME_DIALOG_DUAL_FIELD_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>
#include <utility>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_dual_field : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::DUAL_FIELD;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;
    using pair_list      = std::vector<std::pair<std::string, std::string>>;

public:
    const appearance_ptr appearance;
    const pair_list      pairs;
    const std::string    message;
    const uint16_t       pursuit;
    const uint32_t       oid;

public:
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
    dialog_dual_field(appearance_ptr&& appearance,
                      const pair_list& pairs,
                      std::string_view message,
                      uint32_t         oid     = 0xFFFFFFFD,
                      uint16_t         pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_dual_field<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_dual_field_v550 = dialog_dual_field<CLIENT_VERSION::v550>;
using dialog_dual_field_v565 = dialog_dual_field<CLIENT_VERSION::v565>;
using dialog_dual_field_v651 = dialog_dual_field<CLIENT_VERSION::v651>;
#else
class dialog_dual_field : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::DUAL_FIELD;

public:
    uint8_t                                          type_echo = 0;
    uint32_t                                         oid       = 0;
    std::string                                      message;
    uint16_t                                         pursuit = 0;
    std::vector<std::pair<std::string, std::string>> pairs;

public:
    dialog_dual_field() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
