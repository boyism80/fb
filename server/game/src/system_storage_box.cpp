#include <fb/game/system_storage_box.h>

#include <fb/game/character.h>

using namespace fb::game;

bool system_storage_box::expired(const fb::model::datetime& now) const
{
    return this->expire_date.has_value() && this->expire_date.value() < now;
}

bool system_storage_box::eligible(const character& ch) const
{
    if (this->user != 0 && this->user != ch.id)
        return false;

    return this->created_before.has_value() == false || ch.created_date() < this->created_before.value();
}

storage_box::entry system_storage_box::to_entry() const
{
    auto e                  = storage_box::entry{};
    e.system_storage_box_id = this->id;
    e.title                 = this->title;
    e.message               = this->message;
    e.attachments           = storage_box::attachments_from_json(this->attachments);
    e.received              = false;
    if (this->expire_date.has_value())
        e.expire_date = this->expire_date;

    return e;
}
