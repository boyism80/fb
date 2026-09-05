#ifndef __PROTOCOL_GAME_LIST_H__
#define __PROTOCOL_GAME_LIST_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
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

#ifndef BOT
template <CLIENT_VERSION V>
class dialog_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr             appearance;
    const std::vector<std::string>   lists;
    const std::optional<std::string> message; // nullopt → subtype 3 (no text)
    const bool                       button_prev;
    const uint32_t                   oid;

public:
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

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_list<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_list_v550 = dialog_list<CLIENT_VERSION::v550>;
using dialog_list_v565 = dialog_list<CLIENT_VERSION::v565>;
using dialog_list_v651 = dialog_list<CLIENT_VERSION::v651>;
#else
class dialog_list : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x30;

public:
    uint16_t                   look;
    uint8_t                    color;
    std::optional<std::string> message;
    bool                       button_prev;
    uint8_t                    type_echo;
    uint32_t                   oid;
    std::vector<std::string>   lists;

public:
    dialog_list() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
