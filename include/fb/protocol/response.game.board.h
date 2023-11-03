#ifndef __PROTOCOL_REQUEST_BOARD_H__
#define __PROTOCOL_REQUEST_BOARD_H__

#include <fb/protocol/protocol.h>
#include <fb/game/model.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace board {

class sections : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::list<board_section>        section_list;
#else
    const std::list<board_section>& section_list;
#endif

public:
#ifdef BOT
    sections() : fb::protocol::base::header(0x31)
    { }
#else
    sections(const std::list<board_section>& section_list) : fb::protocol::base::header(0x31), section_list(section_list)
    { }
#endif

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        auto size = (uint16_t)this->section_list.size();

        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
                  .write_u16(size);

        for(auto& section : this->section_list)
        {
            out_stream.write_u16(section.id)
                      .write(section.title);
        }
    }
#else
    void deserialize(fb::istream& in_stream)
    {
        in_stream.read_u8();
        auto size = in_stream.read_u16();
        for(auto i = 0; i < size; i++)
        {
            auto id = in_stream.read_u16();
            auto title = in_stream.readstr_u8();
            this->section_list.push_back(board_section {id, title});
        }
    }
#endif
};


class articles : public fb::protocol::base::header
{
public:
    const board_section&                    section;
    const std::list<board_article_new>&     article_list;

public:
    articles(const board_section& section, const std::list<board_article_new>& article_list) : fb::protocol::base::header(0x31),
        section(section), article_list(article_list)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x02)
                  .write_u8(fb::game::board::button_enabled::NEXT | fb::game::board::button_enabled::WRITE)
                  .write_u16(section.id)
                  .write(section.title);


        auto                    count = this->article_list.size();
        out_stream.write_u8((uint8_t)count);

        for(auto& article : this->article_list)
        {
            out_stream.write_u8(0x00)
                .write_u16(article.id)
                .write(article.uname)
                .write_u8(article.month)
                .write_u8(article.day)
                .write(article.title);
        }

        out_stream.write_u8(0x00);
    }
};

class article : public fb::protocol::base::header
{
public:
    const board_article_new&            value;

public:
    article(const board_article_new& value) : fb::protocol::base::header(0x31),
        value(value)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        //auto                    section = fb::game::model::board.at(section_id);
        //if(section == nullptr)
        //    throw fb::game::board::section::not_found_exception();

        //auto                    article = section->find(article_id);
        //if(article == nullptr)
        //    throw fb::game::board::article::not_found_exception();

        base::header::serialize(out_stream);
        out_stream.write_u8(0x03)
                  .write_u8(fb::game::board::button_enabled::NEXT | fb::game::board::button_enabled::WRITE)
                  .write_u8(0x00)
                  .write_u16(this->value.id)
                  .write(this->value.uname)
                  .write_u8(this->value.month)
                  .write_u8(this->value.day)
                  .write(this->value.title)
                  .write(this->value.contents, true)
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