#ifndef __MAIL_BOX_H__
#define __MAIL_BOX_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>
#include <fb/model/model.h>

namespace fb::game {

using namespace fb::model::enum_value;

class character;

class mail_box
{
public:
    struct summary;
    struct mail;
    struct received;

private:
    uint16_t                     _unread_count = 0;
    std::unordered_set<uint32_t> _system_mail_ids;

public:
    character& owner;

public:
    mail_box(character& owner);
    ~mail_box() = default;

public:
    // clang-format off
    void     show(const std::vector<summary>& summaries, MAIL_BUTTON_ENABLE flag);
    void     show(const mail& mail, MAIL_BUTTON_ENABLE flag);
    void     message(std::string_view message, bool success, BULLETIN_MESSAGE_TYPE action);
    uint16_t unread_count() const;
    void     unread_count(uint16_t value);
    void     init_system_mails(const std::vector<uint32_t>& ids);
    bool     contains_system_mail(uint32_t system_mail_id) const;
    void     mark_system_mail(uint32_t system_mail_id);
    // clang-format on
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

struct mail_box::received
{
    uint32_t                user_id = 0;
    uint16_t                unread  = 0;
    summary                 snapshot;
    std::optional<uint32_t> system_mail_id;
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
