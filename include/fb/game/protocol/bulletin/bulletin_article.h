#ifndef __PROTOCOL_GAME_ARTICLE_H__
#define __PROTOCOL_GAME_ARTICLE_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/bulletin.h>
#else
#include <fb/bot/bot.bulletin.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class bulletin_article : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifndef BOT
    const fb::game::bulletin::article& value;
    const BULLETIN_BUTTON_ENABLE       flag;
#else

#endif

public:
#ifndef BOT
    bulletin_article(const fb::game::bulletin::article& value, BULLETIN_BUTTON_ENABLE flag);
#else

#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const;
#else

#endif
};

} // namespace fb::protocol::game::response

#endif