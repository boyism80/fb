#ifndef __PROTOCOL_GAME_CHAT_H__
#define __PROTOCOL_GAME_CHAT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class chat : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0E;

public:
#ifndef BOT
    bool        shout;
    std::string message;
#else
    const bool        shout;
    const std::string message;
#endif

public:
#ifndef BOT
    chat() = default;
#else
    chat(bool shout, const std::string& message);
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

class chat : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x0D;

public:
#ifndef BOT
    const fb::game::object& me;
    const std::string       text;
    const CHAT_TYPE         type;
#else
    uint32_t    sequence;
    std::string text;
    CHAT_TYPE   type;
#endif

public:
#ifndef BOT
    chat(const fb::game::object& me, const std::string& text, CHAT_TYPE type);
#else
    chat() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif