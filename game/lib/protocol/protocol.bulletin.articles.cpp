#include <fb/game/protocol/bulletin/bulletin_articles.h>

namespace fb::protocol::game::response {

#ifndef BOT
bulletin_articles::bulletin_articles(const fb::model::bulletin&                    bulletin,
                                     const std::list<fb::game::bulletin::article>& article_list,
                                     BULLETIN_BUTTON_ENABLE                        button_flags) :
    bulletin(bulletin),
    article_list(article_list),
    button_flags(button_flags)
{ }
#endif

#ifndef BOT
async::task<void> bulletin_articles::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>(0x02);
    writer.write<uint8_t>(static_cast<uint8_t>(button_flags));
    writer.write<uint16_t>(bulletin.id);
    writer.write<std::string>(bulletin.name);

    auto count = this->article_list.size();
    writer.write<uint8_t>((uint8_t)count);

    for (auto& article : this->article_list)
    {
        writer.write<uint8_t>(0x00);
        writer.write<uint16_t>(article.id);
        writer.write<std::string>(article.uname);
        writer.write<uint8_t>(article.month);
        writer.write<uint8_t>(article.day);
        writer.write<std::string>(article.title);
    }

    writer.write<uint8_t>(0x00);
}
#endif

} // namespace fb::protocol::game::response