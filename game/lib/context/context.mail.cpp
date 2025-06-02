#include <fb/game/context.h>

using namespace fb::game;

void context::assert_mail(uint32_t error) const
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

void context::on_write_mail(const internal_resp::WriteMail& resp)
{
    assert_mail(resp.error);

    this->_shard[resp.mail.user]->ids.lock([&resp](auto& ids) {
        if (ids.contains(resp.mail.user) == false)
            return;

        auto ch     = ids[resp.mail.user];
        std::ignore = ch->thread()->dispatch([ch, unread = resp.unread](auto& thread) -> async::task<void> {
            ch->unread_mail(unread);
            co_return;
        });
    });
}

async::task<internal_resp::WriteMail>
context::send_mail(const character& ch, const std::string& to, const std::string& title, const std::string& contents)
{
    auto   thread = ch.thread();
    auto&& resp   = co_await this->post<internal_reqs::WriteMail, internal_resp::WriteMail>(
        "internal",
        "/mail/write",
        internal_reqs::WriteMail{ch.id(), to, title, contents, config<uint32_t>("id")});
    co_await this->switch_thread(ch);

    this->assert_mail(resp.error);
    this->on_write_mail(resp);
    co_return std::move(resp);
}

async::task<internal_resp::GetMailList> context::mail_list(const character& ch, uint16_t offset, uint16_t count)
{
    auto&& resp = co_await this->get<internal_resp::GetMailList>(
        "internal",
        std::format("/mail/{}?offset={}&count={}", ch.id(), offset, count));
    this->assert_mail(resp.error);
    co_return std::move(resp);
}

async::task<internal_resp::GetMail> context::read_mail(character& ch, uint16_t id)
{
    auto   url    = std::format("/mail/{}/{}", ch.id(), id);
    auto   thread = ch.thread();
    auto&& resp   = co_await this->get<internal_resp::GetMail>("internal", url);
    co_await this->switch_thread(ch);

    this->assert_mail(resp.error);
    ch.unread_mail(resp.unread);
    co_return std::move(resp);
}

async::task<internal_resp::DeleteMail> context::delete_mail(character& ch, uint16_t id)
{
    auto   thread = ch.thread();
    auto&& resp   = co_await this->post<internal_reqs::DeleteMail, internal_resp::DeleteMail>(
        "internal",
        "/mail/delete",
        internal_reqs::DeleteMail{ch.id(), id});
    co_await this->switch_thread(ch);
    this->assert_mail(resp.error);
    ch.unread_mail(resp.unread);
    co_return std::move(resp);
}