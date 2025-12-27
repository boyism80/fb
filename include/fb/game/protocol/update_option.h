#ifndef __PROTOCOL_GAME_CHANGE_OPTION_H__
#define __PROTOCOL_GAME_CHANGE_OPTION_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class update_option : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x1B;

public:
#ifndef BOT
    OPTION option;
    bool   ride = false;
#else
    const OPTION option;
    const bool   ride = false;
#endif

public:
#ifndef BOT
    update_option() = default;
#else
    update_option(OPTION option, bool ride) :
        option(option),
        ride(ride)
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