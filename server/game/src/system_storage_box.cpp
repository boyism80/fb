#include <fb/game/character.h>
#include <fb/game/system_storage_box.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>
#include <sstream>

using namespace fb::game;

system_storage_box::system_storage_box(const fb::protocol::internal::SystemStorageBox& dto) :
    id(dto.id),
    title(dto.title),
    message(dto.message),
    attachments(dto.attachments),
    created_date(fb::model::datetime(dto.created_date))
{
    if (dto.user != 0)
        this->user = dto.user;

    if (dto.expired_date.has_value() && !dto.expired_date.value().empty())
        this->expire_date = fb::model::datetime(dto.expired_date.value());

    if (dto.external_ref.has_value() && !dto.external_ref.value().empty())
        this->external_ref = dto.external_ref.value();
}

bool system_storage_box::expired(const fb::model::datetime& now) const
{
    return this->expire_date.has_value() && this->expire_date.value() < now;
}

bool system_storage_box::eligible_for(const character& ch) const
{
    if (this->user.has_value())
        return ch.id == this->user.value();

    return ch.created_date() < this->created_date;
}

storage_box::entry system_storage_box::to_entry() const
{
    auto e                  = storage_box::entry{};
    e.system_storage_box_id = this->id;
    e.title                 = this->title;
    e.message               = this->message;

    if (!this->attachments.empty())
    {
        auto json   = Json::Value{};
        auto reader = Json::Reader{};
        auto stream = std::istringstream(this->attachments);
        if (reader.parse(stream, json) && json.isArray())
        {
            e.attachments.reserve(json.size());
            for (const auto& item : json)
            {
                e.attachments.emplace_back(item);
            }
        }
    }

    e.received = false;
    if (this->expire_date.has_value())
        e.expire_date = this->expire_date;

    return e;
}
