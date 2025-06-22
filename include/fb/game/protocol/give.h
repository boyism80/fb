#ifndef __PROTOCOL_GAME_GIVE_H__
#define __PROTOCOL_GAME_GIVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class give_item : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x29;

public:
    uint8_t slot = 0;
    bool    all  = false;

public:
    give_item() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

class give_money : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x2A;

public:
    uint32_t money;

public:
    give_money() = default;

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif