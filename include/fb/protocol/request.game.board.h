#ifndef __PROTOCOL_RESPONSE_BOARD_H__
#define __PROTOCOL_RESPONSE_BOARD_H__

#ifdef DELETE
#undef DELETE
#endif

#include <fb/protocol/protocol.h>
#ifndef BOT
#include <board.h>
#endif

namespace fb::protocol::game::request::board {

class board : public fb::protocol::base::header
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
    std::string  contents;
#endif

public:
#ifdef BOT
    board(BOARD_ACTION       action,
          uint16_t           section  = 0,
          uint16_t           article  = 0,
          uint16_t           offset   = 0,
          const std::string& title    = "",
          const std::string& contents = "") :
        action(action),
        section(section),
        article(article),
        offset(offset),
        title(title),
        contents(contents)
    { }
#else
    board() = default;
#endif

public:
#ifdef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
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
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
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
        }
    }
#endif
};

} // namespace fb::protocol::game::request::board

#endif // !__PROTOCOL_RESPONSE_BOARD_H__