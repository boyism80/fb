#ifndef __PROTOCOL_GAME_SECTIONS_H__
#define __PROTOCOL_GAME_SECTIONS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifdef BOT
#include <fb/bot/bulletin.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class bulletin_sections : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x31;

public:
#ifdef BOT
    std::vector<fb::bot::bulletin> bulletins;
#endif

public:
#ifdef BOT
    bulletin_sections() = default;
#else
    bulletin_sections();
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