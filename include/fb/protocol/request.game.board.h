#ifndef __PROTOCOL_RESPONSE_BOARD_H__
#define __PROTOCOL_RESPONSE_BOARD_H__

#ifdef DELETE
#undef DELETE
#endif

#include <fb/protocol/protocol.h>
#include <fb/game/board.h>

namespace fb { namespace protocol { namespace game { namespace request { namespace board {

class board : public fb::protocol::base::header
{
public:
#if BOT
    const BOARD_ACTION              action;
    const uint16_t                  section;
    const uint16_t                  article;
    const uint16_t                  offset;
    const std::string               title;
    const std::string               contents;
#else
    BOARD_ACTION                    action;
    uint16_t                        section;
    uint16_t                        article;
    uint16_t                        offset;
    std::string                     title;
    std::string                     contents;
#endif

public:
#ifdef BOT
    board(BOARD_ACTION action, uint16_t section = 0, uint16_t article = 0, uint16_t offset = 0, const std::string& title = "", const std::string& contents = "") :
        fb::protocol::base::header(0x3B),
        action(action), section(section), article(article), offset(offset), title(title), contents(contents)
    { }
#else
    board() : fb::protocol::base::header(0x3B)
    { }
#endif

public:
#ifdef BOT
    void serialize(fb::ostream& out_stream) const
    {
        fb::protocol::base::header::serialize(out_stream);
        out_stream.write_u8((uint8_t)this->action);
        switch(this->action)
        {
        case BOARD_ACTION::ARTICLES:
            out_stream.write_u16(this->section);
            out_stream.write_u16(this->offset);
            break;

        case BOARD_ACTION::ARTICLE:
            out_stream.write_u16(this->section);
            out_stream.write_u16(this->article);
            break;

        case BOARD_ACTION::WRITE:
            out_stream.write_u16(this->section);
            out_stream.writestr_u8(this->title);
            out_stream.writestr_u16(this->contents);
            break;

        case BOARD_ACTION::DELETE:
            out_stream.write_u16(this->section);
            out_stream.write_u16(this->article);
            break;
        }
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        this->action = (BOARD_ACTION)in_stream.read_u8();

        switch(this->action)
        {
        case BOARD_ACTION::ARTICLES:
            this->section = in_stream.read_u16();
            this->offset = in_stream.read_u16();
            break;

        case BOARD_ACTION::ARTICLE:
            this->section = in_stream.read_u16();
            this->article = in_stream.read_u16();
            break;

        case BOARD_ACTION::WRITE:
            this->section = in_stream.read_u16();
            this->title = in_stream.readstr_u8();
            this->contents = in_stream.readstr_u16();
            break;

        case BOARD_ACTION::DELETE:
            this->section = in_stream.read_u16();
            this->article = in_stream.read_u16();
            break;
        }
    }
#endif
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_BOARD_H__