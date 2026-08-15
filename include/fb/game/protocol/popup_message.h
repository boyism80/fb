#ifndef __PROTOCOL_GAME_POPUP_MESSAGE_H__
#define __PROTOCOL_GAME_POPUP_MESSAGE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class popup_message : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x35;

public:
#ifndef BOT
    const uint8_t     param0;
    const uint8_t     param1;
    const uint8_t     param2;
    const uint8_t     param3;
    const std::string text;
#else
    uint8_t     param0 = 0;
    uint8_t     param1 = 0;
    uint8_t     param2 = 0;
    uint8_t     param3 = 0;
    std::string text;
#endif

public:
#ifndef BOT
    popup_message(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text);
#else
    popup_message() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

class popup_input : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x1B;

public:
#ifndef BOT
    const uint8_t     param0;
    const uint8_t     param1;
    const uint8_t     param2;
    const uint8_t     param3;
    const std::string text;
#else
    uint8_t     param0 = 0;
    uint8_t     param1 = 0;
    uint8_t     param2 = 0;
    uint8_t     param3 = 0;
    std::string text;
#endif

public:
#ifndef BOT
    popup_input(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text);
#else
    popup_input() = default;
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
