#ifndef __BOT_INTEGRATION_BULLETIN_BOT_H__
#define __BOT_INTEGRATION_BULLETIN_BOT_H__

#include <fb/protocol/header.h>
#include <fb/model/model.h>
#include <fb/bot/bulletin.h>

using namespace fb::model::enum_value;

namespace fb::bot::integration {

class bulletin_bot : public fb::protocol::header
{
public:
    static constexpr uint8_t header = 0x31;

public:
    enum class bulletin_type : uint8_t
    {
        sections     = 0x01,
        articles     = 0x02,
        article      = 0x03,
        mails        = 0x04,
        mail         = 0x05,
        message      = 0x06,
        message_mail = 0x07
    };

    struct article_data
    {
        uint16_t    id;
        std::string uname;
        uint8_t     month;
        uint8_t     day;
        std::string title;
    };

    struct mail_data
    {
        bool        unread;
        uint16_t    id;
        std::string sender_name;
        uint8_t     month;
        uint8_t     day;
        std::string title;
    };

    bulletin_type type;

    // bulletin_sections data
    std::vector<fb::bot::bulletin> bulletins;

    // bulletin_articles data
    BULLETIN_BUTTON_ENABLE    articles_button_flags;
    uint16_t                  articles_bulletin_id;
    std::string               articles_bulletin_name;
    std::vector<article_data> articles;

    // bulletin_article data
    BULLETIN_BUTTON_ENABLE article_flag;
    uint16_t               article_id;
    std::string            article_uname;
    uint8_t                article_month;
    uint8_t                article_day;
    std::string            article_title;
    std::string            article_contents;

    // bulletin_mails data
    MAIL_BUTTON_ENABLE     mails_button_flags;
    std::string            mails_mail_name;
    std::vector<mail_data> mails;

    // bulletin_mail data
    MAIL_BUTTON_ENABLE mail_flag;
    uint16_t           mail_id;
    std::string        mail_sender_name;
    uint8_t            mail_month;
    uint8_t            mail_day;
    std::string        mail_title;
    std::string        mail_contents;

    // bulletin_message data
    std::string message_text;
    bool        message_success;
    bool        message_mail;

public:
    bulletin_bot() = default;

public:
    [[nodiscard]] async::task<void> deserialize(fb::stream_reader<big_endian>& reader);
};

} // namespace fb::bot::integration

#endif