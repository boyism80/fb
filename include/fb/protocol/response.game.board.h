#ifndef __PROTOCOL_REQUEST_BOARD_H__
#define __PROTOCOL_REQUEST_BOARD_H__

#include <fb/protocol/protocol.h>
#include <fb/game/model.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace board {

class sections : public fb::protocol::base::header
{
public:
#ifdef BOT
    std::list<fb::model::board&>       section_list;
#else
    const fb::model::container& model;
#endif

public:
#ifdef BOT
    sections() : fb::protocol::base::header(0x31)
#else
    sections(const fb::model::container& model) : fb::protocol::base::header(0x31), model(model)
#endif
    { }

public:
#ifndef BOT
    void serialize(fb::ostream& out_stream) const
    {
        auto size = this->model.board.size();

        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
                  .write_u16(size);

        for(const auto& [k, v] : this->model.board)
        {
            out_stream.write_u16(k)
                      .write(v.name);
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
            this->section_list.push_back(fb::model::board{id, title});
        }
    }
#endif
};


class articles : public fb::protocol::base::header
{
public:
    const fb::model::board&                 section;
    const std::list<fb::game::board::article>&      article_list;
    const BOARD_BUTTON_ENABLE           button_flags;

public:
    articles(const fb::model::board& section, const std::list<fb::game::board::article>& article_list, BOARD_BUTTON_ENABLE button_flags) : fb::protocol::base::header(0x31),
        section(section), article_list(article_list), button_flags(button_flags)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x02)
                  .write_u8(button_flags)
                  .write_u16(section.id)
                  .write(section.name);

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
    const fb::game::board::article&            value;
    const BOARD_BUTTON_ENABLE      button_flags;

public:
    article(const fb::game::board::article& value, BOARD_BUTTON_ENABLE button_flags) : fb::protocol::base::header(0x31),
        value(value), button_flags(button_flags)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x03)
                  .write_u8(button_flags)
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
    const bool              deleted;
    const bool              refresh;

public:
    message(const std::string& text, bool deleted, bool refresh = false) : fb::protocol::base::header(0x31),
        text(text), deleted(deleted), refresh(refresh)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(this->refresh ? 0x06 : 0x07)     // mail 관련 0x06인 것 같다. 확인 필요
                  .write_u8(this->deleted)
                  .write(this->text)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_REQUEST_BOARD_H__