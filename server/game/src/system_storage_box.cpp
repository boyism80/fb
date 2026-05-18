#include <fb/game/system_storage_box.h>
#include <fb/protocol/flatbuffer/protocol.h>

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
