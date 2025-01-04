#ifndef __PROTOCOL_GAME_SECTIONS_H__
#define __PROTOCOL_GAME_SECTIONS_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifdef BOT
#include <fb/bot/bot.board.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class board_sections : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifdef BOT
    std::vector<fb::bot::board> boards;
#else
    const fb::model::model& model;
#endif

public:
#ifdef BOT
    board_sections() = default;
#else
    board_sections(const fb::model::model& model);
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