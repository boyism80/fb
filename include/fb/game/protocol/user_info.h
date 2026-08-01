#ifndef __PROTOCOL_GAME_USER_INFO_H__
#define __PROTOCOL_GAME_USER_INFO_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <array>
#include <string>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x4D — opens USERINFO.EPF survey / profile dialog.
 * type == 2: prefills 8 CP949 strings (4 text + 4 combo labels that must match client options).
 * Other type values open an empty form (no string payload).
 */
class user_info : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x4D;

public:
#ifndef BOT
    const uint8_t                    type;
    const std::array<std::string, 8> strings;
#else
    uint8_t                    type = 0;
    std::array<std::string, 8> strings;
#endif

public:
#ifndef BOT
    explicit user_info(uint8_t type, const std::array<std::string, 8>& strings) :
        type(type),
        strings(strings)
    { }
#else
    user_info() = default;
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
