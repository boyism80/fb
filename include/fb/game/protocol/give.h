#ifndef __PROTOCOL_GAME_GIVE_H__
#define __PROTOCOL_GAME_GIVE_H__

#include <fb/protocol/header.h>
#include <fb/protocol/client_version.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

template <CLIENT_VERSION V>
class give_item : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x29;
    FB_PROTOCOL_VERSION_TAGS(V);

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
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

template <CLIENT_VERSION V>
class give_money : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x2A;
    FB_PROTOCOL_VERSION_TAGS(V);

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
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::request

#endif