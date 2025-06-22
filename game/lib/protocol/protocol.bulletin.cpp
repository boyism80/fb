#include <fb/game/protocol/bulletin/bulletin.h>

namespace fb::protocol::game::request {

#ifndef BOT
async::task<void> bulletin::deserialize(fb::stream_reader<big_endian>& reader)
{
    co_await header::deserialize(reader);
    this->action = (BULLETIN_ACTION)reader.read<uint8_t>();

    switch (this->action)
    {
    case BULLETIN_ACTION::ARTICLES:
        this->section = reader.read<uint16_t>();
        this->offset  = reader.read<uint16_t>();
        break;

    case BULLETIN_ACTION::ARTICLE:
        this->section = reader.read<uint16_t>();
        this->article = reader.read<uint16_t>();
        break;

    case BULLETIN_ACTION::WRITE:
        this->section  = reader.read<uint16_t>();
        this->title    = reader.read<std::string, uint8_t>();
        this->contents = reader.read<std::string, uint16_t>();
        break;

    case BULLETIN_ACTION::DELETE:
        this->section = reader.read<uint16_t>();
        this->article = reader.read<uint16_t>();
        break;

    case BULLETIN_ACTION::SEND_MAIL:
        this->section  = reader.read<uint16_t>();
        this->user     = reader.read<std::string>();
        this->title    = reader.read<std::string>();
        this->contents = reader.read<std::string, uint16_t>();
        break;
    }
}
#else
bulletin::bulletin(BULLETIN_ACTION    action,
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

async::task<void> bulletin::serialize(fb::stream_writer<big_endian>& writer) const
{
    co_await header::serialize(writer);
    writer.write<uint8_t>(header);
    writer.write<uint8_t>((uint8_t)this->action);
    switch (this->action)
    {
    case BULLETIN_ACTION::ARTICLES:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->offset);
        break;

    case BULLETIN_ACTION::ARTICLE:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->article);
        break;

    case BULLETIN_ACTION::WRITE:
        writer.write<uint16_t>(this->section);
        writer.write<std::string, uint8_t>(this->title);
        writer.write<std::string, uint16_t>(this->contents);
        break;

    case BULLETIN_ACTION::DELETE:
        writer.write<uint16_t>(this->section);
        writer.write<uint16_t>(this->article);
        break;
    }
}
#endif
} // namespace fb::protocol::game::request