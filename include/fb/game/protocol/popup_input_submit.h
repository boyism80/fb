#ifndef __PROTOCOL_GAME_POPUP_INPUT_SUBMIT_H__
#define __PROTOCOL_GAME_POPUP_INPUT_SUBMIT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#include <string>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class popup_input_submit : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x23;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT
    const uint8_t     param0;
    const std::string text;
#else
    uint8_t     param0 = 0;
    std::string text;
#endif

public:
#ifdef BOT
    popup_input_submit(uint8_t param0, const std::string& text);
#else
    popup_input_submit() = default;
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif
