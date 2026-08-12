#ifndef __PROTOCOL_GAME_CHAT_H__
#define __PROTOCOL_GAME_CHAT_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/object.h>
#endif
#include <string_view>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class chat : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0E;
    FB_PROTOCOL_VERSION_TAGS(V);

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
    chat(bool shout, std::string_view message);
#endif

public:
#ifdef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class chat : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x0D;

public:
#ifndef BOT
    const fb::game::object& me;
    const std::string       text;
    const CHAT_TYPE         type;
#else
    uint32_t    oid;
    std::string text;
    CHAT_TYPE   type;
#endif

public:
#ifndef BOT
    chat(const fb::game::object& me, std::string_view text, CHAT_TYPE type);
#else
    chat() = default;
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