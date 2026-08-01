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

void mail_box::message(std::string_view message, bool success, BULLETIN_MESSAGE_TYPE action)
{
    this->owner.assert_thread();
    this->owner.listener.on_show_bulletin_message(this->owner, message, success, action);
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

void mail_box::init_system_mails(const std::vector<uint32_t>& ids)
{
    this->owner.assert_thread();
    this->_system_mail_ids.clear();
    this->_system_mail_ids.insert(ids.begin(), ids.end());
}

bool mail_box::contains_system_mail(uint32_t system_mail_id) const
{
    this->owner.assert_thread();
    return this->_system_mail_ids.contains(system_mail_id);
}

void mail_box::mark_system_mail(uint32_t system_mail_id)
{
    this->owner.assert_thread();
    this->_system_mail_ids.insert(system_mail_id);
}