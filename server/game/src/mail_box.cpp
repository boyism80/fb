#include <fb/game/mail_box.h>

#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

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

async::task<void> mail_box::send(std::string_view to, std::string_view title, std::string_view contents)
{
    auto&  server       = this->owner.server;
    auto   weak         = this->owner.weak_from_this();
    auto   to_str       = std::string(to);
    auto   title_str    = std::string(title);
    auto   contents_str = std::string(contents);
    auto   world        = this->owner.world();
    auto&& resp         = co_await server.http.post(
        weak,
        "internal",
        "/mail/write",
        internal_reqs::WriteMail{world, this->owner.id, to_str, title_str, contents_str, fb::config<uint32_t>("id")});
    if (weak.expired())
        throw std::runtime_error("character expired while sending mail");

    server.mail.on_error(resp.error);

    auto snapshot         = service::mail::to_summary(resp.mail);
    snapshot.created_date = "";
    co_await server.mail.on_received(mail_box::received{
        .user_id        = resp.mail.user,
        .unread         = resp.unread,
        .snapshot       = snapshot,
        .system_mail_id = resp.mail.system_mail_id,
    });
}

async::task<std::vector<mail_box::summary>> mail_box::list(uint16_t offset, uint16_t count) const
{
    auto&  server = this->owner.server;
    auto   weak   = this->owner.weak_from_this();
    auto   world  = this->owner.world();
    auto&& resp   = co_await server.http.get<internal_resp::GetMailList>(
        weak,
        "internal",
        std::format("/mail/{}/{}?offset={}&count={}", world, this->owner.id, offset, count));
    if (weak.expired())
        throw std::runtime_error("character expired while listing mail");

    server.mail.on_error(resp.error);

    auto summaries = std::vector<mail_box::summary>{};
    summaries.reserve(resp.summary_list.size());
    for (const auto& row : resp.summary_list)
    {
        summaries.push_back(service::mail::to_summary(row));
    }

    co_return summaries;
}

async::task<mail_box::mail> mail_box::read(uint16_t id)
{
    auto&  server = this->owner.server;
    auto   weak   = this->owner.weak_from_this_as<character>();
    auto   world  = this->owner.world();
    auto   url    = std::format("/mail/{}/{}/{}", world, this->owner.id, id);
    auto&& resp   = co_await server.http.get<internal_resp::GetMail>(weak, "internal", url);
    auto   ptr    = weak.lock();
    if (ptr == nullptr)
        throw std::runtime_error("character expired while reading mail");

    server.mail.on_error(resp.error);
    ptr->mail_box.unread_count(resp.unread);
    co_return service::mail::to_mail(resp.mail);
}

async::task<void> mail_box::remove(uint16_t id)
{
    auto&  server = this->owner.server;
    auto   weak   = this->owner.weak_from_this_as<character>();
    auto   world  = this->owner.world();
    auto&& resp   = co_await server.http.post(weak,
                                            "internal",
                                            "/mail/delete",
                                            internal_reqs::DeleteMail{world, this->owner.id, id});
    auto   ptr    = weak.lock();
    if (ptr == nullptr)
        throw std::runtime_error("character expired while deleting mail");

    server.mail.on_error(resp.error);
    ptr->mail_box.unread_count(resp.unread);
    server.system_mail.join(ptr->id);
    co_return;
}
