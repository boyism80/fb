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
    static constexpr uint8_t header = 0x31;

public:
#ifndef BOT
    const fb::model::bulletin&                    bulletin;
    const std::list<fb::game::bulletin::article>& article_list;
    const BULLETIN_BUTTON_ENABLE                  button_flags;
#else
    struct article_data
    {
        uint16_t    id;
        std::string uname;
        uint8_t     month;
        uint8_t     day;
        std::string title;
    };

    BULLETIN_BUTTON_ENABLE    button_flags;
    uint16_t                  bulletin_id;
    std::string               bulletin_name;
    std::vector<article_data> articles;
#endif

public:
#ifndef BOT
    bulletin_articles(const fb::model::bulletin& bulletin, const std::list<fb::game::bulletin::article>& article_list, BULLETIN_BUTTON_ENABLE button_flags);
#else
    bulletin_articles() = default;
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