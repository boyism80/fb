#include <fb/game/protocol/board/board.h>

namespace fb::protocol::game::request {

#ifdef BOT
board::board(BOARD_ACTION       action,
             uint16_t           section,
             uint16_t           article,
             uint16_t           offset,
             const std::string& title,
             const std::string& contents) :
    action(action),
    section(section),
    article(article),
    offset(offset),
    title(title),
    contents(contents)
{ }
#endif

#ifdef BOT
async::task<void> board::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>((uint8_t)this->action);
    switch (this->action)
    {
    case BOARD_ACTION::ARTICLES:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->offset);
        break;

    case BOARD_ACTION::ARTICLE:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->article);
        break;

    case BOARD_ACTION::WRITE:
        writer.write<uint16_t>(this->section);
        writer.write<std::string, uint8_t>(this->title);
        writer.write<std::string, uint16_t>(this->contents);
        break;

    case BOARD_ACTION::DELETE:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->article);
        break;
    }
}
#else
async::task<void> board::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = (BOARD_ACTION)reader.read<uint8_t>();

    switch (this->action)
    {
    case BOARD_ACTION::ARTICLES:
        this->section = reader.read<uint16_t>();
        this->offset  = reader.read<uint16_t>();
        break;

    case BOARD_ACTION::ARTICLE:
        this->section = reader.read<uint16_t>();
        this->article = reader.read<uint16_t>();
        break;

    case BOARD_ACTION::WRITE:
        this->section  = reader.read<uint16_t>();
        this->title    = reader.read<std::string, uint8_t>();
        this->contents = reader.read<std::string, uint16_t>();
        break;

    case BOARD_ACTION::DELETE:
        this->section = reader.read<uint16_t>();
        this->article = reader.read<uint16_t>();
        break;

    case BOARD_ACTION::SEND_MAIL:
        this->section  = reader.read<uint16_t>();
        this->user     = reader.read<std::string>();
        this->title    = reader.read<std::string>();
        this->contents = reader.read<std::string, uint16_t>();
        break;
    }
}
#endif
} // namespace fb::protocol::game::request
