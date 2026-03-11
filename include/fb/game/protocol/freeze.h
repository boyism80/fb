#ifndef __PROTOCOL_GAME_FREEZE_H__
#define __PROTOCOL_GAME_FREEZE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class freeze : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x51;

public:
#ifndef BOT
    const bool enable;
#else
    bool enable;
#endif

public:
#ifndef BOT
    explicit freeze(bool enable) :
        enable(enable)
    { }
#else
    freeze() = default;
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif
