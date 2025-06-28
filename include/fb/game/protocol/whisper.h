#ifndef __GAME_PROTOCOL_WHISPER_H__
#define __GAME_PROTOCOL_WHISPER_H__

#include <fb/protocol/header.h>

namespace fb::protocol::game::request {

class whisper : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x19;

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
    whisper(const std::string& name, const std::string& message) :
        name(name),
        message(message)
    { }
#else
    whisper() = default;
#endif

public:
#ifdef BOT // bot only
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif