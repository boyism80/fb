#ifndef __PROTOCOL_GAME_GROUP_H__
#define __PROTOCOL_GAME_GROUP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class group : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x2E;

public:
#ifndef BOT
    std::string name;
#else
    const std::string name;
#endif

public:
#ifndef BOT
    group() = default;
#else
    group(const std::string& name) :
        name(name)
    { }
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