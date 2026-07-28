#ifndef __PROTOCOL_GAME_POPUP_MESSAGE_H__
#define __PROTOCOL_GAME_POPUP_MESSAGE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

/**
 * S2C 0x35 — timed popup message (mode 1: resource 96, long timer).
 * Wire: param0..3 + u16 BE text_len + CP949 text.
 * param0 is reserved (presenter does not consume it); prefer 0.
 * param1 = height factor (16 * (param1 + 2)); param2 = width factor (16 * (param2 + 3));
 * param3 != 0 enables center-width recalculation from string length.
 * Long popup does not submit on dismiss (no C2S).
 */
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
    popup_message(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text) :
        param0(param0),
        param1(param1),
        param2(param2),
        param3(param3),
        text(std::string(text))
    { }
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

/**
 * S2C 0x1B — editable short popup (mode 0: resource 95).
 * Same wire shape as popup_message. Distinct from C2S update_option (also 0x1B).
 * Presenter consumes wire param2 as height and param3 as width (param1 is overwritten).
 * On dismiss the client submits edited text via C2S popup_input_submit (0x23).
 * Lua: ch:popup_input(...) yields until submit, then resumes with the text (like ch:input).
 */
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
    popup_input(uint8_t param0, uint8_t param1, uint8_t param2, uint8_t param3, std::string_view text) :
        param0(param0),
        param1(param1),
        param2(param2),
        param3(param3),
        text(std::string(text))
    { }
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
