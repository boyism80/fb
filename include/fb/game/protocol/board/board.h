#ifndef __PROTOCOL_GAME_BOARD_H__
#define __PROTOCOL_GAME_BOARD_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>

#ifdef DELETE
#undef DELETE
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::request {

class board : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x3B;

public:
#if BOT
    const BOARD_ACTION action;
    const uint16_t     section;
    const uint16_t     article;
    const uint16_t     offset;
    const std::string  title;
    const std::string  contents;
#else
    BOARD_ACTION action;
    uint16_t     section;
    uint16_t     article;
    uint16_t     offset;
    std::string  title;
    std::string  user;
    std::string  contents;
#endif

public:
#ifdef BOT
    board(BOARD_ACTION       action,
          uint16_t           section  = 0,
          uint16_t           article  = 0,
          uint16_t           offset   = 0,
          const std::string& title    = "",
          const std::string& contents = "");
#else
    board() = default;
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