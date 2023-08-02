#ifndef __PROTOCOL_REQUEST_BOARD_H__
#define __PROTOCOL_REQUEST_BOARD_H__

#include <fb/protocol/protocol.h>
#include <fb/game/data_set.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace board {

class sections : public fb::protocol::base::header
{
public:
    sections() : fb::protocol::base::header(0x31)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto& sections = fb::game::data_set::board.sections();
        auto size = (uint16_t)sections.size();

        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
                  .write_u16(size);

        for(int i = 0; i < size; i++)
        {
            out_stream.write_u16(i)
                      .write(sections[i]->title());
        }
    }
};


class articles : public fb::protocol::base::header
{
public:
    const uint16_t              section_id;
    const uint16_t              offset;

public:
    articles(uint16_t section_id, uint16_t offset) : fb::protocol::base::header(0x31),
        section_id(section_id), offset(offset)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto&             section = fb::game::data_set::board.sections()[section_id];

        base::header::serialize(out_stream);
        out_stream.write_u8(0x02)
                  .write_u8(fb::game::board::button_enabled::NEXT | fb::game::board::button_enabled::WRITE)
                  .write_u16(section_id)
                  .write(section->title());


        auto offset = this->offset;
        if(offset == 0x7FFF)
            offset = uint16_t(section->size() - 1);
        auto                    reverse_offset = uint16_t(section->size() - (offset + 1));

        auto                    count = std::min(size_t(20), section->size() - reverse_offset);
        out_stream.write_u8((uint8_t)count);

        for(auto i = section->rbegin() + reverse_offset; i != section->rend(); i++)
        {
            const auto          article = i->get();
            if(article->deleted())
                continue;

            out_stream.write_u8(0x00)
                .write_u16(article->id())
                .write(article->writer())
                .write_u8(article->month())
                .write_u8(article->day())
                .write(article->title());

            if(--count == 0)
                break;
        }

        out_stream.write_u8(0x00);
    }
};

class article : public fb::protocol::base::header
{
public:
    const uint16_t              section_id;
    const uint16_t              article_id;
    const fb::game::session&    me;

public:
    article(uint16_t section_id, uint16_t article_id, const fb::game::session& me) : fb::protocol::base::header(0x31),
        section_id(section_id), article_id(article_id), me(me)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        auto                    section = fb::game::data_set::board.at(section_id);
        if(section == nullptr)
            throw fb::game::board::section::not_found_exception();

        auto                    article = section->find(article_id);
        if(article == nullptr)
            throw fb::game::board::article::not_found_exception();

        base::header::serialize(out_stream);
        out_stream.write_u8(0x03)
                  .write_u8(fb::game::board::button_enabled::NEXT | fb::game::board::button_enabled::WRITE)
                  .write_u8(0x00)
                  .write_u16(this->article_id)
                  .write(article->writer())
                  .write_u8(article->month())
                  .write_u8(article->day())
                  .write(article->title())
                  .write(article->content(), true)
                  .write_u8(0x00);
    }
};


class message : public fb::protocol::base::header
{
public:
    const std::string       text;
    const bool              success;
    const bool              refresh;

public:
    message(const std::string& text, bool success, bool refresh = false) : fb::protocol::base::header(0x31),
        text(text), success(success), refresh(refresh)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->refresh ? 0x06 : 0x07)     // mail 관련 0x06인 것 같다. 확인 필요
                  .write_u8(this->success)
                  .write(this->text)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_BOARD_H__