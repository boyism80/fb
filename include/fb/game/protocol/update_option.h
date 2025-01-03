#ifndef __PROTOCOL_GAME_CHANGE_OPTION_H__
#define __PROTOCOL_GAME_CHANGE_OPTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class update_option : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x1B;

public:
    OPTION option;
    bool   ride = false;

public:
    update_option() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif