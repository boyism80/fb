#ifndef __MAIL_BOX_H__
#define __MAIL_BOX_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <fb/model/model.h>

namespace fb::game {

using namespace fb::model::enum_value;

class character;

class mail_box
{
public:
    struct summary;
    struct mail;

private:
    uint16_t _unread_count = 0;

public:
    character& owner;

public:
    mail_box(character& owner);
    ~mail_box() = default;

public:
    void     show(const std::vector<summary>& summaries, MAIL_BUTTON_ENABLE flag);
    void     show(const mail& mail, MAIL_BUTTON_ENABLE flag);
    void     message(std::string_view message, bool success, BULLETIN_MESSAGE_TYPE action);
    uint16_t unread_count() const;
    void     unread_count(uint16_t value);
};

struct mail_box::summary
{
    uint16_t    id           = 0;
    uint32_t    user         = 0;
    std::string sender       = "";
    bool        read         = false;
    std::string title        = "";
    std::string created_date = "";
};

struct mail_box::mail
{
    uint16_t    id           = 0;
    uint32_t    user         = 0;
    std::string sender       = "";
    std::string title        = "";
    std::string contents     = "";
    bool        read         = false;
    std::string created_date = "";
};

} // namespace fb::game

#endif