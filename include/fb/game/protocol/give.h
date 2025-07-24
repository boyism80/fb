#ifndef __PROTOCOL_GAME_GIVE_H__
#define __PROTOCOL_GAME_GIVE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class give_item : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x29;

public:
#ifndef BOT
    uint8_t slot = 0;
    bool    all  = false;
#else
    const uint8_t slot = 0;
    const bool    all  = false;
#endif

public:
#ifndef BOT
    give_item() = default;
#else
    give_item(uint8_t slot, bool all) :
        slot(slot),
        all(all)
    { }
#endif

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
    static constexpr uint8_t header = 0x2A;

public:
#ifndef BOT
    uint32_t money;
#else
    const uint32_t money;
#endif

public:
#ifndef BOT
    give_money() = default;
#else
    give_money(uint32_t money) :
        money(money)
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