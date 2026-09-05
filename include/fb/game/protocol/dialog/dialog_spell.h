#ifndef __PROTOCOL_GAME_DIALOG_SPELL_H__
#define __PROTOCOL_GAME_DIALOG_SPELL_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_spell : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::SPELL;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr appearance;
    const std::string    message;
    const uint16_t       pursuit;
    const uint32_t       oid;

public:
    dialog_spell(const fb::model::object& obj,
                 std::string_view         message,
                 uint32_t                 oid     = 0xFFFFFFFD,
                 uint16_t                 pursuit = 0xFFFF);
    dialog_spell(const fb::game::object& object,
                 std::string_view        message,
                 uint32_t                oid     = 0xFFFFFFFD,
                 uint16_t                pursuit = 0xFFFF);
    dialog_spell(appearance_ptr&& appearance,
                 std::string_view message,
                 uint32_t         oid     = 0xFFFFFFFD,
                 uint16_t         pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_spell<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_spell_v550 = dialog_spell<CLIENT_VERSION::v550>;
using dialog_spell_v565 = dialog_spell<CLIENT_VERSION::v565>;
using dialog_spell_v651 = dialog_spell<CLIENT_VERSION::v651>;
#else
class dialog_spell : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::SPELL;

public:
    uint8_t     type_echo = 0;
    uint32_t    oid       = 0;
    std::string message;
    uint16_t    pursuit = 0;

public:
    dialog_spell() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
