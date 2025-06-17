#ifndef __PROTOCOL_GAME_ARTICLES_H__
#define __PROTOCOL_GAME_ARTICLES_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#ifndef BOT
#include <fb/game/bulletin.h>
#endif

using namespace fb::model::enum_value;

namespace fb::protocol::game::response {

class bulletin_articles : public fb::protocol::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifndef BOT
    const fb::model::bulletin&                    bulletin;
    const std::list<fb::game::bulletin::article>& article_list;
    const BULLETIN_BUTTON_ENABLE                  button_flags;
#else

#endif

public:
#ifndef BOT
    bulletin_articles(const fb::model::bulletin&                    bulletin,
                      const std::list<fb::game::bulletin::article>& article_list,
                      BULLETIN_BUTTON_ENABLE                        button_flags);
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