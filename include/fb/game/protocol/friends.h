#ifndef __PROTOCOL_GAME_FRIENDS_H__
#define __PROTOCOL_GAME_FRIENDS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class friends : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x77;

public:
#ifdef BOT
    const std::vector<std::string> names;
#else
    std::vector<std::string> names;
#endif

public:
#ifdef BOT
    friends(const std::vector<std::string>& names);
#else
    friends() = default;
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif