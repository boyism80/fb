#ifndef __GAME_PROTOCOL_WHISPER_H__
#define __GAME_PROTOCOL_WHISPER_H__

#include <fb/protocol/header.h>

namespace fb::protocol::game::request {

class whisper : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x19;

public:
    std::string name;
    std::string message;

public:
    whisper() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif