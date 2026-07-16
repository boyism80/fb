#ifndef __PROTOCOL_GAME_ARTICLE_H__
#define __PROTOCOL_GAME_ARTICLE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/bulletin.h>
#else
#include <fb/bot/bulletin.h>
#endif

namespace fb::protocol::game::response {

using namespace fb::model::enum_value;

class bulletin_article : public fb::protocol::header
{
public:
    static constexpr uint8_t opcode = 0x31;

public:
#ifndef BOT
    const fb::game::bulletin::article& value;
    const BULLETIN_BUTTON_ENABLE       flag;
#else
    BULLETIN_BUTTON_ENABLE flag;
    uint16_t               id;
    std::string            uname;
    uint8_t                month;
    uint8_t                day;
    std::string            title;
    std::string            contents;
#endif

public:
#ifndef BOT
    bulletin_article(const fb::game::bulletin::article& value, BULLETIN_BUTTON_ENABLE flag);
#else
    bulletin_article() = default;
#endif

public:
#ifndef BOT
    void serialize(fb::stream_writer<big_endian>& writer) const;
#else
    void deserialize(fb::stream_reader<big_endian>& reader);
#endif
};

} // namespace fb::protocol::game::response

#endif