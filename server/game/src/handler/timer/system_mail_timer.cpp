#include <fb/game/handler/timer/system_mail_timer.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/system_mail.h>
#include <fb/logger.h>
#include <algorithm>
#include <format>

using namespace fb::game::handler::timer;

namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

bool system_mail_timer::is_expired(const fb::game::system_mail& mail, const fb::model::datetime& now)
{
    return mail.expire_date.has_value() && mail.expire_date.value() < now;
}

void system_mail_timer::prune_expired_mails(std::vector<fb::game::system_mail>& mails, const fb::model::datetime& now)
{
    mails.erase(std::remove_if(mails.begin(),
                               mails.end(),
                               [&](const fb::game::system_mail& mail) {
                                   return is_expired(mail, now);
                               }),
                mails.end());
}

system_mail_timer::system_mail_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server),
    _delivery{delivery_coroutine()}
{ }

async::task<void> system_mail_timer::handle()
{
    std::ignore = co_await this->_delivery.next();
    co_return;
}

std::vector<std::tuple<uint32_t, fb::model::datetime>> system_mail_timer::collect_online_users() const
{
    auto users = std::vector<std::tuple<uint32_t, fb::model::datetime>>{};

    this->server.characters.read([&users](const character::container& container) {
        users.reserve(container.size());

        for (auto it = container.cbegin(); it != container.cend(); ++it)
        {
            const auto& ch = it->second;
            if (ch == nullptr)
                continue;

            users.emplace_back(ch->id, ch->created_date());
        }
    });

    return users;
}

fb::async_generator<void> system_mail_timer::delivery_coroutine()
{
    auto offset = uint32_t{0};
    auto mails  = std::vector<fb::game::system_mail>{};

    while (true)
    {
        auto        max_mail_id = uint32_t{0};
        const auto  world       = fb::config<uint32_t>("world");
        const auto& fetch_url   = std::format("/mail/system/{}?offset={}", world, offset);

        try
        {
            auto&& resp = co_await this->server.http.get<internal_resp::GetSystemMails>("internal", fetch_url);
            if (resp.error == 0)
            {
                const auto now = this->server.now();
                for (const auto& dto : resp.mails)
                {
                    auto mail = fb::game::system_mail(dto);
                    if (is_expired(mail, now))
                        continue;

                    max_mail_id = std::max(max_mail_id, mail.id);

                    const auto already_tracked =
                        std::any_of(mails.cbegin(), mails.cend(), [&](const fb::game::system_mail& existing) {
                            return existing.id == mail.id;
                        });
                    if (!already_tracked)
                        mails.push_back(std::move(mail));
                }
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Failed to fetch system mails: {}", e.what());
        }

        if (max_mail_id > 0)
            offset = max_mail_id + 1;

        const auto now = this->server.now();
        prune_expired_mails(mails, now);

        const auto online_users = this->collect_online_users();

        for (const auto& mail : mails)
        {
            if (is_expired(mail, now))
                continue;

            auto eligible = std::vector<uint32_t>{};
            eligible.reserve(online_users.size());

            for (const auto& [user_id, created_date] : online_users)
            {
                if (created_date < mail.created_date)
                    eligible.push_back(user_id);
            }

            for (std::size_t i = 0; i < eligible.size(); i += chunk_limit)
            {
                const auto end = std::min(i + chunk_limit, eligible.size());

                auto chunk_users = std::vector<uint32_t>{};
                chunk_users.assign(eligible.begin() + static_cast<std::ptrdiff_t>(i),
                                   eligible.begin() + static_cast<std::ptrdiff_t>(end));

                try
                {
                    auto&& resp =
                        co_await this->server.http.post("internal",
                                                        "/mail/deliver-system-mail",
                                                        internal_reqs::DeliverSystemMail{world,
                                                                                         mail.id,
                                                                                         mail.sender,
                                                                                         std::move(chunk_users),
                                                                                         mail.title,
                                                                                         mail.contents,
                                                                                         fb::config<uint32_t>("id")});

                    if (resp.error != 0)
                        fb::logger::warn("DeliverSystemMail failed for system mail {}: error {}", mail.id, resp.error);
                }
                catch (const std::exception& e)
                {
                    fb::logger::warn("DeliverSystemMail request failed for system mail {}: {}", mail.id, e.what());
                }

                co_await fb::async_suspend{};
            }
        }

        co_await fb::async_suspend{};
    }
}
