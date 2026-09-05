#ifndef __PROTOCOL_GAME_SLOT_H__
#define __PROTOCOL_GAME_SLOT_H__

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

using namespace fb::model::enum_value;

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_slot : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::SLOT;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr       appearance;
    const std::vector<uint8_t> slots;
    const std::string          message;
    const uint32_t             oid;

public:
    dialog_slot(const fb::model::object&    obj,
                const std::vector<uint8_t>& slots,
                std::string_view            message,
                uint32_t                    oid = 0xFFFFFFFD);
    dialog_slot(const fb::game::object&     object,
                const std::vector<uint8_t>& slots,
                std::string_view            message,
                uint32_t                    oid = 0xFFFFFFFD);
    dialog_slot(appearance_ptr&&            appearance,
                const std::vector<uint8_t>& slots,
                std::string_view            message,
                uint32_t                    oid = 0xFFFFFFFD);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_slot<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_slot_v550 = dialog_slot<CLIENT_VERSION::v550>;
using dialog_slot_v565 = dialog_slot<CLIENT_VERSION::v565>;
using dialog_slot_v651 = dialog_slot<CLIENT_VERSION::v651>;
#else
class dialog_slot : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::SLOT;

public:
    uint16_t             look;
    uint8_t              color;
    std::vector<uint8_t> slots;
    std::string          message;
    uint8_t              type_echo;
    uint32_t             oid;

public:
    dialog_slot() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
