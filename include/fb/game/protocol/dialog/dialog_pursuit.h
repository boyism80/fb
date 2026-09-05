#ifndef __PROTOCOL_GAME_DIALOG_PURSUIT_H__
#define __PROTOCOL_GAME_DIALOG_PURSUIT_H__

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
class dialog_pursuit : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::PURSUIT;
    FB_PROTOCOL_VERSION_TAGS(V);
    using appearance_ptr = std::unique_ptr<fb::game::appearance>;

public:
    const appearance_ptr           appearance;
    const std::vector<std::string> options;
    const std::string              message;
    const uint16_t                 pursuit;
    const uint32_t                 oid;

public:
    dialog_pursuit(const fb::model::object&        obj,
                   const std::vector<std::string>& options,
                   std::string_view                message,
                   uint32_t                        oid     = 0xFFFFFFFD,
                   uint16_t                        pursuit = 0xFFFF);
    dialog_pursuit(const fb::game::object&         object,
                   const std::vector<std::string>& options,
                   std::string_view                message,
                   uint32_t                        oid     = 0xFFFFFFFD,
                   uint16_t                        pursuit = 0xFFFF);
    dialog_pursuit(appearance_ptr&&                appearance,
                   const std::vector<std::string>& options,
                   std::string_view                message,
                   uint32_t                        oid     = 0xFFFFFFFD,
                   uint16_t                        pursuit = 0xFFFF);

public:
    void serialize(fb::stream_writer<big_endian>& writer) const;
};

template <>
void dialog_pursuit<CLIENT_VERSION::v651>::serialize(fb::stream_writer<big_endian>& writer) const;

using dialog_pursuit_v550 = dialog_pursuit<CLIENT_VERSION::v550>;
using dialog_pursuit_v565 = dialog_pursuit<CLIENT_VERSION::v565>;
using dialog_pursuit_v651 = dialog_pursuit<CLIENT_VERSION::v651>;
#else
class dialog_pursuit : public fb::protocol::header
{
public:
    static constexpr uint8_t                opcode = 0x2F;
    static constexpr fb::game::dialog::type type   = fb::game::dialog::type::PURSUIT;

public:
    uint8_t                  type_echo = 0;
    uint32_t                 oid       = 0;
    std::string              message;
    uint16_t                 pursuit = 0;
    std::vector<std::string> options;

public:
    dialog_pursuit() = default;

public:
    void deserialize(fb::stream_reader<big_endian>& reader);
};
#endif

} // namespace fb::protocol::game::response

#endif
