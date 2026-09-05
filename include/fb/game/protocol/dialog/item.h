#ifndef __PROTOCOL_GAME_ITEM_H__
#define __PROTOCOL_GAME_ITEM_H__

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
class dialog_item : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::ITEM;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr                appearance;
    const fb::game::dialog::item_pairs& items;
    const std::string                   message;
    const uint16_t                      pursuit;
    const uint32_t                      oid;

public:
    dialog_item(const fb::model::object&            obj,
                const fb::game::dialog::item_pairs& items,
                std::string_view                    message,
                uint32_t                            oid     = 0xFFFFFFFD,
                uint16_t                            pursuit = 0xFFFF);
    dialog_item(const fb::game::object&             object,
                const fb::game::dialog::item_pairs& items,
                std::string_view                    message,
                uint32_t                            oid     = 0xFFFFFFFD,
                uint16_t                            pursuit = 0xFFFF);
    dialog_item(appearance_ptr&&                    appearance,
                const fb::game::dialog::item_pairs& items,
                std::string_view                    message,
                uint32_t                            oid     = 0xFFFFFFFD,
                uint16_t                            pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_item<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_item_v550 = dialog_item<CLIENT_VERSION::v550>;
using dialog_item_v565 = dialog_item<CLIENT_VERSION::v565>;
using dialog_item_v651 = dialog_item<CLIENT_VERSION::v651>;
#else
class dialog_item : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::ITEM;

public:
    struct item_data
    {
        uint16_t    look;
        uint8_t     color;
        uint32_t    value;
        std::string name;
        std::string desc;
    };

    uint16_t               look;
    uint8_t                color;
    std::string            message;
    uint16_t               pursuit;
    uint8_t                type_echo;
    uint32_t               oid;
    std::vector<item_data> items;

public:
    dialog_item() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
