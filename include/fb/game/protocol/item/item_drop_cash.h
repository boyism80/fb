#ifndef __PROTOCOL_GAME_DROP_CASH_H__
#define __PROTOCOL_GAME_DROP_CASH_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class item_drop_cash : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x24;

public:
    uint32_t chunk;

public:
    item_drop_cash() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::protocol::game::request

#endif