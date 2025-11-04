#ifndef __MAIL_BOX_H__
#define __MAIL_BOX_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::game {

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
    void     message(const std::string& message, bool success, bool unknown = true);
    uint16_t unread_count() const;
    void     unread_count(uint16_t value);
};

struct mail_box::summary
{
    uint16_t    id           = 0;
    uint32_t    user         = 0;
    uint32_t    sender       = 0;
    std::string sender_name  = "";
    bool        read         = false;
    std::string title        = "";
    std::string created_date = "";
};

struct mail_box::mail
{
    uint16_t    id           = 0;
    uint32_t    user         = 0;
    uint32_t    sender       = 0;
    std::string sender_name  = "";
    std::string title        = "";
    std::string contents     = "";
    bool        read         = false;
    std::string created_date = "";
};

} // namespace fb::game

#endif