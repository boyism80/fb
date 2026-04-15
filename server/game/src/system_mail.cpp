#include <fb/game/system_mail.h>
#include <fb/protocol/flatbuffer/protocol.h>

using namespace fb::game;

system_mail::system_mail(const fb::protocol::internal::SystemMail& dto) :
    id(dto.id),
    sender(dto.sender),
    title(dto.title),
    contents(dto.contents),
    created_date(fb::model::datetime(dto.created_date))
{
    if (dto.expire_date.has_value() && !dto.expire_date.value().empty())
        this->expire_date = fb::model::datetime(dto.expire_date.value());
    else
        this->expire_date = std::nullopt;
}
