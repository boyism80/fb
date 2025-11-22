#include <fb/game/mail_box.h>
#include <fb/game/character.h>

using namespace fb::game;

mail_box::mail_box(character& owner) :
    owner(owner)
{ }

void mail_box::show(const std::vector<summary>& summaries, MAIL_BUTTON_ENABLE flag)
{
    this->owner.assert_thread();
    this->owner.listener.on_show_mail_box(this->owner, summaries, flag);
}
void mail_box::show(const mail& mail, MAIL_BUTTON_ENABLE flag)
{
    this->owner.assert_thread();
    this->owner.listener.on_show_mail_box(this->owner, mail, flag);
}

void mail_box::message(const std::string& message, bool success, bool unknown)
{
    this->owner.assert_thread();
    this->owner.listener.on_show_bulletin_message(this->owner, message, success, unknown);
}

uint16_t mail_box::unread_count() const
{
    this->owner.assert_thread();
    return this->_unread_count;
}

void mail_box::unread_count(uint16_t value)
{
    this->owner.assert_thread();
    if (this->_unread_count != value)
    {
        this->_unread_count = value;
        this->owner.update(UPDATE_STATE_LEVEL::MINIMUM);
    }
}

void mail_box::add_system_mail_user(uint32_t mail_id, const std::optional<std::string>& expire_date)
{
    this->owner.assert_thread();
    auto it = this->_system_mail_users.find(mail_id);
    if (it == this->_system_mail_users.end())
    {
        this->_system_mail_users[mail_id] = system_mail_user{mail_id, false, expire_date};
    }
    else
    {
        // Update expire_date if it changed
        it->second.expire_date = expire_date.has_value() ? std::make_optional<fb::model::datetime>(expire_date.value()) : std::nullopt;
    }
}

void mail_box::update_system_mail_user_read(uint32_t mail_id, bool read)
{
    this->owner.assert_thread();
    auto it = this->_system_mail_users.find(mail_id);
    if (it != this->_system_mail_users.end())
    {
        it->second.read = read;
    }
}

bool mail_box::try_mark_system_mail_user_as_sent(uint32_t mail_id)
{
    this->owner.assert_thread();
    auto it = this->_system_mail_users.find(mail_id);
    if (it != this->_system_mail_users.end())
    {
        // Atomically check and set: only mark as sent if not already sent
        if (!it->second.read)
        {
            it->second.read = true;
            return true; // Successfully marked as sent (was not sent before)
        }
        return false; // Already sent
    }
    return false; // Mail not found
}

const std::map<uint32_t, system_mail_user>& mail_box::get_system_mail_users() const
{
    this->owner.assert_thread();
    return this->_system_mail_users;
}