#include <fb/game/service/mail.h>

#include <fb/encoding.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <json/json.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace fb;
using namespace fb::game;

service::mail::mail(fb::game::server& server) :
    server(server)
{ }

void service::mail::on_error(uint32_t error) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::NOT_FOUND_CHARACTER:
        throw std::runtime_error(_TEXT(MESSAGE_INVALID_USER_NAME));

    case ERROR_CODE::NOT_FOUND_MAIL:
    case ERROR_CODE::MAIL_NOT_EXISTS:
        throw std::runtime_error(_TEXT(MESSAGE_NOT_FOUND_MAIL));

    case ERROR_CODE::MAIL_BOX_FULL:
        throw std::runtime_error(_TEXT(MESSAGE_MAIL_BOX_FULL));

    default:
        throw std::runtime_error(std::format(_TEXT(MESSAGE_UNKNOWN_ERROR_WITH_CODE), error));
    }
}

mail_box::summary service::mail::to_summary(const fb::protocol::internal::MailSummary& mail)
{
    return mail_box::summary{
        .id           = mail.id,
        .user         = mail.user,
        .sender       = mail.sender,
        .read         = mail.read,
        .title        = mail.title,
        .created_date = mail.created_date,
    };
}

mail_box::summary service::mail::to_summary(const fb::protocol::internal::Mail& mail)
{
    return mail_box::summary{
        .id           = mail.id,
        .user         = mail.user,
        .sender       = mail.sender,
        .read         = mail.read,
        .title        = mail.title,
        .created_date = mail.created_date,
    };
}

mail_box::mail service::mail::to_mail(const fb::protocol::internal::Mail& mail)
{
    return mail_box::mail{
        .id           = mail.id,
        .user         = mail.user,
        .sender       = mail.sender,
        .title        = mail.title,
        .contents     = mail.contents,
        .read         = mail.read,
        .created_date = mail.created_date,
    };
}

async::task<void> service::mail::on_received(const mail_box::received& entry)
{
    auto ch = this->server.characters.find(entry.user_id);
    if (ch != nullptr)
    {
        auto weak    = ch->template weak_from_this_as<character>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [this, ch, entry](auto&) -> async::task<void> {
            ch->mail_box.unread_count(entry.unread);
            if (entry.system_mail_id.has_value())
                ch->mail_box.mark_system_mail(entry.system_mail_id.value());

            auto log_data            = Json::Value();
            log_data["character_id"] = static_cast<Json::Int64>(ch->id);
            log_data["sender_name"]  = UTF8(entry.snapshot.sender, PLATFORM::WINDOWS);
            log_data["mail_id"]      = static_cast<Json::Int64>(entry.snapshot.id);
            log_data["title"]        = UTF8(entry.snapshot.title, PLATFORM::WINDOWS);
            this->server.log.write("mail_receive", log_data);
            co_return;
        };
        builder.enqueue();
    }
    co_return;
}
