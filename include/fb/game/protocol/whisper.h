#ifndef __GAME_PROTOCOL_WHISPER_H__
#define __GAME_PROTOCOL_WHISPER_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <string_view>

namespace fb::protocol::game::request {

template <CLIENT_VERSION V>
class whisper : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x19;
    FB_PROTOCOL_VERSION_TAGS(V);

public:
#ifdef BOT // bot only
    const std::string name;
    const std::string message;
#else
    std::string name;
    std::string message;
#endif

public:
#ifdef BOT // bot only
    whisper(std::string_view name, std::string_view message) :
        name(std::string(name)),
        message(std::string(message))
    { }
#else
    whisper() = default;
#endif

public:
#ifdef BOT // bot only
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif