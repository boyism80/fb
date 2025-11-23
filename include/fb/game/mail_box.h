#ifndef __MAIL_BOX_H__
#define __MAIL_BOX_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <map>
#include <fb/model/model.h>
#include <fb/game/system_mail_user.h>

using namespace fb::model::enum_value;

namespace fb::game {

class character;

class mail_box
{
public:
    struct summary;
    struct mail;

private:
    uint16_t                             _unread_count = 0;
    std::map<uint32_t, system_mail_user> _system_mail_users;

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

    // System mail user management
    void                                        add_system_mail_user(uint32_t mail_id, const std::optional<std::string>& expire_date);
    void                                        update_system_mail_user_read(uint32_t mail_id, bool read);
    bool                                        try_mark_system_mail_user_as_sent(uint32_t mail_id);
    const std::map<uint32_t, system_mail_user>& get_system_mail_users() const;
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