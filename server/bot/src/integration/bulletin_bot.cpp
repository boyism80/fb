#include <fb/bot/integration/bulletin_bot.h>

namespace fb::bot::integration {

void bulletin_bot::deserialize(fb::stream_reader<big_endian>& reader)
{
    header::deserialize(reader);
    uint8_t type_byte = reader.read<uint8_t>();
    this->type        = static_cast<bulletin_type>(type_byte);

    switch (this->type)
    {
    case bulletin_type::sections:
    {
        auto size = reader.read<uint16_t>();
        this->bulletins.clear();
        for (auto i = 0; i < size; i++)
        {
            auto id    = reader.read<uint16_t>();
            auto title = reader.read<std::string, uint8_t>();
            this->bulletins.push_back(fb::bot::bulletin(id, title));
        }
    }
    break;

    case bulletin_type::articles:
    {
        this->articles_button_flags  = static_cast<BULLETIN_BUTTON_ENABLE>(reader.read<uint8_t>());
        this->articles_bulletin_id   = reader.read<uint16_t>();
        this->articles_bulletin_name = reader.read<std::string, uint8_t>();

        uint8_t count = reader.read<uint8_t>();
        this->articles.clear();

        for (int i = 0; i < count; i++)
        {
            article_data article;
            reader.read<uint8_t>(); // 0x00
            article.id    = reader.read<uint16_t>();
            article.uname = reader.read<std::string, uint8_t>();
            article.month = reader.read<uint8_t>();
            article.day   = reader.read<uint8_t>();
            article.title = reader.read<std::string, uint8_t>();
            this->articles.push_back(article);
        }

        reader.read<uint8_t>(); // 0x00
    }
    break;

    case bulletin_type::article:
    {
        this->article_flag = static_cast<BULLETIN_BUTTON_ENABLE>(reader.read<uint8_t>());
        reader.read<uint8_t>(); // 0x00
        this->article_id       = reader.read<uint16_t>();
        this->article_uname    = reader.read<std::string, uint8_t>();
        this->article_month    = reader.read<uint8_t>();
        this->article_day      = reader.read<uint8_t>();
        this->article_title    = reader.read<std::string, uint8_t>();
        this->article_contents = reader.read<std::string, uint16_t>();
        reader.read<uint8_t>(); // 0x00
    }
    break;

    case bulletin_type::mails:
    {
        this->mails_button_flags = static_cast<MAIL_BUTTON_ENABLE>(reader.read<uint8_t>());
        reader.read<uint16_t>(); // 0xFFFF
        this->mails_mail_name = reader.read<std::string, uint8_t>();

        uint8_t count = reader.read<uint8_t>();
        this->mails.clear();

        for (int i = 0; i < count; i++)
        {
            mail_data mail;
            mail.unread      = reader.read<bool>();
            mail.id          = reader.read<uint16_t>();
            mail.sender_name = reader.read<std::string, uint8_t>();
            mail.month       = reader.read<uint8_t>();
            mail.day         = reader.read<uint8_t>();
            mail.title       = reader.read<std::string, uint8_t>();
            this->mails.push_back(mail);
        }

        reader.read<uint8_t>(); // 0x00
    }
    break;

    case bulletin_type::mail:
    {
        this->mail_flag = static_cast<MAIL_BUTTON_ENABLE>(reader.read<uint8_t>());
        reader.read<uint8_t>(); // 0x00
        this->mail_id          = reader.read<uint16_t>();
        this->mail_sender_name = reader.read<std::string, uint8_t>();
        this->mail_month       = reader.read<uint8_t>();
        this->mail_day         = reader.read<uint8_t>();
        this->mail_title       = reader.read<std::string, uint8_t>();
        this->mail_contents    = reader.read<std::string, uint16_t>();
        reader.read<uint8_t>(); // 0x00
    }
    break;

    case bulletin_type::message:
    case bulletin_type::message_mail:
    {
        this->message_mail    = (this->type == bulletin_type::message_mail);
        this->message_success = reader.read<uint8_t>();
        this->message_text    = reader.read<std::string, uint8_t>();
        reader.read<uint8_t>(); // 0x00
    }
    break;

    default:
        throw std::runtime_error("Unknown bulletin type: " + std::to_string(type_byte));
    }
}

} // namespace fb::bot::integration