#include <fb/game/service/mail.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/encoding.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <json/json.h>

using namespace fb;
using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

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
        throw std::runtime_error(_TEXT(MESSAGE_NOT_FOUND_MAIL));

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

void service::mail::apply_received(character& ch, uint16_t unread, const mail_box::summary& snapshot)
{
    ch.mail_box.unread_count(unread);

    auto log_data            = Json::Value();
    log_data["character_id"] = static_cast<Json::Int64>(ch.id);
    log_data["sender_name"]  = UTF8(snapshot.sender, PLATFORM::WINDOWS);
    log_data["mail_id"]      = static_cast<Json::Int64>(snapshot.id);
    log_data["title"]        = UTF8(snapshot.title, PLATFORM::WINDOWS);
    this->server.log.write("mail_receive", log_data);
}

async::task<void> service::mail::on_received(uint32_t user_id, uint16_t unread, const mail_box::summary& snapshot)
{
    auto ch = this->server.characters.find(user_id);
    if (ch != nullptr)
    {
        auto weak    = ch->template weak_from_this_as<character>();
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [this, ch, unread, snapshot](auto&) -> async::task<void> {
            this->apply_received(*ch, unread, snapshot);
            co_return;
        };
        builder.enqueue();
    }
    co_return;
}

async::task<void> service::mail::on_received_batch(const std::vector<mail_box::summary>& snapshots,
                                                   const std::vector<uint32_t>&          user_ids,
                                                   const std::vector<uint16_t>&          unread_counts)
{
    if (snapshots.empty())
        co_return;

    const auto count = snapshots.size();
    for (size_t i = 0; i < count; ++i)
    {
        auto ch = this->server.characters.find(user_ids[i]);
        if (ch == nullptr)
            continue;

        auto weak     = ch->template weak_from_this_as<character>();
        auto unread   = unread_counts[i];
        auto snapshot = snapshots[i];
        auto builder  = this->server.threads.new_builder(weak);
        builder.func  = [this, ch, unread, snapshot](auto&) -> async::task<void> {
            this->apply_received(*ch, unread, snapshot);
            co_return;
        };
        builder.enqueue();
    }
    co_return;
}

async::task<void>
service::mail::send(character& sender, std::string_view to, std::string_view title, std::string_view contents)
{
    auto   weak         = sender.weak_from_this();
    auto   to_str       = std::string(to);
    auto   title_str    = std::string(title);
    auto   contents_str = std::string(contents);
    auto   world        = fb::config<uint32_t>("world");
    auto&& resp         = co_await this->server.http.post(
        "internal",
        "/mail/write",
        internal_reqs::WriteMail{world, sender.id, to_str, title_str, contents_str, fb::config<uint32_t>("id")});
    co_await this->server.threads.switching(weak);
    if (weak.expired())
        throw std::runtime_error("character expired while sending mail");

    this->on_error(resp.error);

    auto snapshot         = to_summary(resp.mail);
    snapshot.created_date = "";
    co_await this->on_received(resp.mail.user, resp.unread, snapshot);
}

async::task<std::vector<mail_box::summary>> service::mail::list(const character& ch, uint16_t offset, uint16_t count)
{
    auto   weak  = ch.weak_from_this();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp  = co_await this->server.http.get<internal_resp::GetMailList>(
        "internal",
        std::format("/mail/{}/{}?offset={}&count={}", world, ch.id, offset, count));
    co_await this->server.threads.switching(weak);
    if (weak.expired())
        throw std::runtime_error("character expired while listing mail");

    this->on_error(resp.error);

    auto summaries = std::vector<mail_box::summary>{};
    summaries.reserve(resp.summary_list.size());
    for (const auto& summary : resp.summary_list)
    {
        summaries.push_back(to_summary(summary));
    }

    co_return summaries;
}

async::task<mail_box::mail> service::mail::read(character& ch, uint16_t id)
{
    auto   weak  = ch.weak_from_this_as<character>();
    auto   world = fb::config<uint32_t>("world");
    auto   url   = std::format("/mail/{}/{}/{}", world, ch.id, id);
    auto&& resp  = co_await this->server.http.get<internal_resp::GetMail>("internal", url);
    co_await this->server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr == nullptr)
        throw std::runtime_error("character expired while reading mail");

    this->on_error(resp.error);
    ptr->mail_box.unread_count(resp.unread);
    co_return to_mail(resp.mail);
}

async::task<void> service::mail::remove(character& ch, uint16_t id)
{
    auto   weak  = ch.weak_from_this_as<character>();
    auto   world = fb::config<uint32_t>("world");
    auto&& resp =
        co_await this->server.http.post("internal", "/mail/delete", internal_reqs::DeleteMail{world, ch.id, id});
    co_await this->server.threads.switching(weak);
    auto ptr = weak.lock();
    if (ptr == nullptr)
        throw std::runtime_error("character expired while deleting mail");

    this->on_error(resp.error);
    ptr->mail_box.unread_count(resp.unread);
    co_return;
}
