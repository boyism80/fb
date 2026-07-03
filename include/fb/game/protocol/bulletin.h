#ifndef __PROTOCOL_GAME_BULLETIN_H__
#define __PROTOCOL_GAME_BULLETIN_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <string_view>

#ifdef DELETE
#undef DELETE
#endif

namespace fb::protocol::game::request {

using namespace fb::model::enum_value;

class bulletin : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x3B;

public:
#if BOT
    const BULLETIN_ACTION action;
    const uint16_t        section;
    const uint16_t        article;
    const uint16_t        offset;
    const std::string     title;
    const std::string     contents;
    const std::string     user;
#else
    BULLETIN_ACTION action;
    uint16_t        section;
    uint16_t        article;
    uint16_t        offset;
    std::string     title;
    std::string     user;
    std::string     contents;
#endif

public:
#ifdef BOT
    bulletin(BULLETIN_ACTION  action,
             uint16_t         section  = 0,
             uint16_t         article  = 0,
             uint16_t         offset   = 0,
             std::string_view title    = "",
             std::string_view contents = "",
             std::string_view user     = "");
#else
    bulletin() = default;
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