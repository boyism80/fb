#ifndef __PROTOCOL_REQUEST_BOARD_H__
#define __PROTOCOL_REQUEST_BOARD_H__

#include <fb/protocol/protocol.h>
#include <fb/model/model.h>
#ifdef BOT
#include <bot.board.h>
#endif

namespace fb::protocol::game::response::board {

class sections : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifdef BOT
    std::vector<fb::bot::board> boards;
#else
    const fb::model::model& model;
#endif

public:
#ifdef BOT
    sections() = default;
#else
    sections(const fb::model::model& model) :
        model(model)
    { }
#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        auto size = this->model.board.size();

        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x01);
        writer.write<uint16_t>(size);

        for (const auto& [k, v] : this->model.board)
        {
            writer.write<uint16_t>(k);
            writer.write<std::string>(v.name);
        }
    }
#else
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader)
    {
        co_await header::deserialize(reader);
        reader.read<uint8_t>();
        auto size = reader.read<uint16_t>();
        for (auto i = 0; i < size; i++)
        {
            auto id    = reader.read<uint16_t>();
            auto title = reader.read<std::string, uint8_t>();

            this->boards.push_back(fb::bot::board(id, title));
        }
    }
#endif
};

class articles : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifndef BOT
    const fb::model::board&                    board;
    const std::list<fb::game::board::article>& article_list;
    const BOARD_BUTTON_ENABLE                  button_flags;
#else

#endif

public:
#ifndef BOT
    articles(const fb::model::board&                    board,
             const std::list<fb::game::board::article>& article_list,
             BOARD_BUTTON_ENABLE                        button_flags) :
        board(board),
        article_list(article_list),
        button_flags(button_flags)
    { }
#else

#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x02);
        writer.write<uint8_t>(static_cast<uint8_t>(button_flags));
        writer.write<uint16_t>(board.id);
        writer.write<std::string>(board.name);

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
#else

#endif
};

class article : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
#ifndef BOT
    const fb::game::board::article& value;
    const BOARD_BUTTON_ENABLE       button_flags;
#else

#endif

public:
#ifndef BOT
    article(const fb::game::board::article& value, BOARD_BUTTON_ENABLE button_flags) :
        value(value),
        button_flags(button_flags)
    { }
#else

#endif

public:
#ifndef BOT
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x03);
        writer.write<uint8_t>(static_cast<uint8_t>(button_flags));
        writer.write<uint8_t>(0x00);
        writer.write<uint16_t>(this->value.id);
        writer.write<std::string>(this->value.uname);
        writer.write<uint8_t>(this->value.month);
        writer.write<uint8_t>(this->value.day);
        writer.write<std::string>(this->value.title);
        writer.write<std::string, uint16_t>(this->value.contents);
        writer.write<uint8_t>(0x00);
    }
#else

#endif
};

class message : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const std::string text;
    const bool        success;
    const bool        mail;

public:
    message(const std::string& text, bool success, bool mail = false) :
        text(text),
        success(success),
        mail(mail)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(this->mail ? 0x06 : 0x07);
        writer.write<uint8_t>(this->success);
        writer.write<std::string>(this->text);
        writer.write<uint8_t>(0x00);
    }
};

class mails : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const MAIL_BUTTON_ENABLE button_flags;

public:
    mails(MAIL_BUTTON_ENABLE button_flags) :
        button_flags(button_flags)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x04);
        writer.write<uint8_t>(static_cast<uint8_t>(this->button_flags));
        writer.write<uint16_t>(65535);
        writer.write<std::string>("mail name");

        auto count = 23;
        writer.write<uint8_t>(count);
        for (int i = 0; i < count; i++)
        {
            auto read = (i % 2) == 0;
            writer.write<bool>(read);
            writer.write<uint16_t>(i + 100); // id
            writer.write<std::string>(std::format("name {}", i));
            writer.write<uint8_t>(12); // month
            writer.write<uint8_t>(31); // day
            writer.write<std::string>(std::format("mail title {}", i));
        }
        writer.write<uint8_t>(0x00);
    }
};

class mail : public fb::protocol::base::header
{
public:
    inline static uint8_t header = 0x31;

public:
    const uint8_t button_flag;

public:
    mail(uint8_t button_flag) :
        button_flag(button_flag)
    { }

public:
    [[nodiscard]] async::task<void> serialize(fb::stream_writer<big_endian>& writer) const
    {
        co_await header::serialize(writer);
        writer.write<uint8_t>(header);
        writer.write<uint8_t>(0x05);
        writer.write<uint8_t>(button_flag);
        writer.write<uint8_t>(0x00);

        auto mail_id = 100;
        writer.write<uint16_t>(mail_id);

        writer.write<std::string>("name");
        writer.write<uint8_t>(12); // month
        writer.write<uint8_t>(31); // day
        writer.write<std::string>("title");
        writer.write<std::string, uint16_t>("mail contents");
        writer.write<uint8_t>(0x00);
    }
};

} // namespace fb::protocol::game::response::board

#endif // !__PROTOCOL_REQUEST_BOARD_H__