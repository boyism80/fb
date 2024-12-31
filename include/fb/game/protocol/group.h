#ifndef __PROTOCOL_GAME_GROUP_H__
#define __PROTOCOL_GAME_GROUP_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class group : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x2E;

public:
    std::string name;

public:
    group() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif