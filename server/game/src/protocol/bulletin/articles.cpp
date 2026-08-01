#include <fb/game/protocol/bulletin/articles.h>

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
void bulletin_articles::serialize(fb::stream_writer<big_endian>& writer) const
{
    header::serialize(writer);
    writer.write<uint8_t>(opcode);
    writer.write<uint8_t>(SUBTYPE_ARTICLE_LIST);
    writer.write<uint8_t>(static_cast<uint8_t>(button_flags));
    writer.write<uint16_t>(bulletin.id);
    writer.write<std::string>(bulletin.name);

    auto count = this->article_list.size();
    writer.write<uint8_t>((uint8_t)count);

    for (auto& article : this->article_list)
    {
        writer.write<uint8_t>(0x00); // article flag (stored/cleared; not rendered)
        writer.write<uint16_t>(article.id);
        writer.write<std::string>(article.uname);
        writer.write<uint8_t>(article.month);
        writer.write<uint8_t>(article.day);
        writer.write<std::string>(article.title);
    }
}
#else
void bulletin_articles::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    reader.read<uint8_t>(); // SUBTYPE_ARTICLE_LIST
    this->button_flags  = static_cast<BULLETIN_BUTTON_ENABLE>(reader.read<uint8_t>());
    this->bulletin_id   = reader.read<uint16_t>();
    this->bulletin_name = reader.read<std::string, uint8_t>();

    uint8_t count = reader.read<uint8_t>();
    this->articles.clear();

    for (int i = 0; i < count; i++)
    {
        article_data article;
        reader.read<uint8_t>(); // article flag
        article.id    = reader.read<uint16_t>();
        article.uname = reader.read<std::string, uint8_t>();
        article.month = reader.read<uint8_t>();
        article.day   = reader.read<uint8_t>();
        article.title = reader.read<std::string, uint8_t>();
        this->articles.push_back(article);
    }
}
#endif

} // namespace fb::protocol::game::response