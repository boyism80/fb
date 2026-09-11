#ifndef __PROTOCOL_GAME_DIALOG_BUY_H__
#define __PROTOCOL_GAME_DIALOG_BUY_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/dialog.h>
#else
#include <fb/game/dialog_type.h>
#endif
#include <string_view>
#include <optional>
#include <vector>

namespace fb::protocol::game::response {

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_buy : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::BUY;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr                 appearance;
    const fb::game::dialog::buy_entries& entries;
    const std::string                    message;
    const uint16_t                       pursuit;
    const uint32_t                       oid;

public:
    dialog_buy(const fb::model::object&             obj,
               const fb::game::dialog::buy_entries& entries,
               std::string_view                     message,
               uint32_t                             oid     = 0xFFFFFFFD,
               uint16_t                             pursuit = 0xFFFF);
    dialog_buy(const fb::game::object&              object,
               const fb::game::dialog::buy_entries& entries,
               std::string_view                     message,
               uint32_t                             oid     = 0xFFFFFFFD,
               uint16_t                             pursuit = 0xFFFF);
    dialog_buy(appearance_ptr&&                     appearance,
               const fb::game::dialog::buy_entries& entries,
               std::string_view                     message,
               uint32_t                             oid     = 0xFFFFFFFD,
               uint16_t                             pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_buy<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_buy_v550 = dialog_buy<CLIENT_VERSION::v550>;
using dialog_buy_v565 = dialog_buy<CLIENT_VERSION::v565>;
using dialog_buy_v651 = dialog_buy<CLIENT_VERSION::v651>;
#else
class dialog_buy : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::BUY;

public:
    struct entry_data
    {
        uint32_t    id    = 0;
        uint16_t    look  = 0;
        uint8_t     color = 0;
        uint32_t    price = 0;
        uint8_t     count = 0;
        std::string name;
        uint8_t     percent = 0xFF;
        std::string value; // 5.50 second string (price as text)
    };

    uint8_t                 type_echo = 0;
    uint32_t                oid       = 0;
    std::string             message;
    uint16_t                pursuit = 0;
    std::vector<entry_data> entries;

public:
    dialog_buy() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
