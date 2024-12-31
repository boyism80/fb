#ifndef __PROTOCOL_GAME_SAVE_H__
#define __PROTOCOL_GAME_SAVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class save : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x21;

public:
    save() = default;

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
};

} // namespace fb::protocol::game::response

#endif