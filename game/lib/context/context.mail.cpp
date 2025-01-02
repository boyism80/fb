#include <fb/game/context.h>

using namespace fb::game;

void context::assert_mail(uint32_t error) const
{
    switch (static_cast<ERROR_CODE>(error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::NOT_FOUND_CHARACTER:
        std::runtime_error("없는사람한테 메일 보내려고함");
        return;

    case ERROR_CODE::NOT_FOUND_MAIL:
        std::runtime_error("메일이 없습니다.");
        return;

    default:
        throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", error));
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