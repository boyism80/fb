#include <fb/game/service/system_mail.h>
#include <fb/game/server.h>
#include <fb/game/character.h>
#include <fb/config.h>
#include <fb/amqp_route.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>
#include <algorithm>
#include <format>
#include <mutex>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

fb::game::system_mail service::system_mail::from_system_mail_dto(const fb::protocol::internal::SystemMail& dto)
{
    auto mail = fb::game::system_mail{
        .id           = dto.id,
        .sender       = dto.sender,
        .title        = dto.title,
        .contents     = dto.contents,
        .created_date = fb::model::datetime(dto.created_date),
    };

    if (dto.expire_date.has_value() && !dto.expire_date.value().empty())
        mail.expire_date = fb::model::datetime(dto.expire_date.value());

    return mail;
}

bool service::system_mail::expired(const fb::game::system_mail& mail, const fb::model::datetime& now)
{
    return mail.expire_date.has_value() && mail.expire_date.value() < now;
}

void service::system_mail::prune_expired_mails(std::vector<fb::game::system_mail>& mails,
                                               const fb::model::datetime&          now)
{
    mails.erase(std::remove_if(mails.begin(),
                               mails.end(),
                               [&](const fb::game::system_mail& entry) {
                                   return expired(entry, now);
                               }),
                mails.end());
}

service::system_mail::system_mail(fb::game::server& server) :
    server(server),
    _delivery(delivery_coroutine())
{ }

async::task<bool> service::system_mail::create(uint32_t                          sender,
                                               std::string_view                  title,
                                               std::string_view                  contents,
                                               const std::optional<std::string>& expire_date)
{
    const auto world = fb::config<uint32_t>("world");
    auto&&     resp  = co_await this->server.http.post(
        "internal",
        "/mail/system",
        internal_reqs::WriteSystemMail{world,
                                       sender,
                                       std::string(title),
                                       std::string(contents),
                                       expire_date.has_value() ? expire_date.value() : std::string{}});

    co_return resp.error == 0;
}

async::task<void> service::system_mail::poll_and_deliver()
{
    std::ignore = this->_delivery.next();
    co_return;
}

fb::async_generator<void> service::system_mail::delivery_coroutine()
{
    while (true)
    {
        const auto now = this->server.now();
        prune_expired_mails(this->_pending_mails, now);

        if (fb::is_cross())
        {
            co_await fb::async_suspend{};
            continue;
        }

        if (this->server.characters.size() == 0)
        {
            co_await fb::async_suspend{};
            continue;
        }

        auto        max_mail_id = uint32_t{0};
        const auto  world       = fb::config<uint32_t>("world");
        const auto& fetch_url   = std::format("/mail/system/{}?offset={}", world, this->_poll_offset);

        try
        {
            auto&& resp = co_await this->server.http.get<internal_resp::GetSystemMails>("internal", fetch_url);
            if (resp.error == 0)
            {
                for (const auto& dto : resp.mails)
                {
                    auto mail = from_system_mail_dto(dto);
                    if (expired(mail, now))
                        continue;

                    max_mail_id = std::max(max_mail_id, mail.id);

                    const auto already_tracked = std::any_of(this->_pending_mails.cbegin(),
                                                             this->_pending_mails.cend(),
                                                             [&](const fb::game::system_mail& existing) {
                                                                 return existing.id == mail.id;
                                                             });
                    if (!already_tracked)
                        this->_pending_mails.push_back(std::move(mail));
                }
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Failed to fetch system mails: {}", e.what());
        }

        if (max_mail_id > 0)
            this->_poll_offset = max_mail_id + 1;

        for (const auto& mail : this->_pending_mails)
        {
            if (expired(mail, now))
                continue;

            auto eligible = std::make_shared<std::vector<uint32_t>>();
            auto mutex    = std::make_shared<std::mutex>();
            co_await this->server.characters.foreach_async(
                [eligible, mutex, mail_id = mail.id, created = mail.created_date](auto& ch) -> async::task<void> {
                    if (ch->created_date() < created && !ch->mail_box.contains_system_mail(mail_id))
                    {
                        auto _ = std::lock_guard(*mutex);
                        eligible->push_back(ch->id);
                    }
                    co_return;
                });

            if (eligible->empty())
                continue;

            for (std::size_t i = 0; i < eligible->size(); i += chunk_limit)
            {
                const auto end = std::min(i + chunk_limit, eligible->size());

                auto chunk_users = std::vector<uint32_t>{};
                chunk_users.assign(eligible->begin() + static_cast<std::ptrdiff_t>(i),
                                   eligible->begin() + static_cast<std::ptrdiff_t>(end));
                const auto chunk_users_to_mark = chunk_users;

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
                    {
                        fb::logger::warn("DeliverSystemMail failed for system mail {}: error {}", mail.id, resp.error);
                    }
                    else
                    {
                        for (auto user_id : chunk_users_to_mark)
                        {
                            auto ch = this->server.characters.find(user_id);
                            if (ch == nullptr)
                                continue;

                            auto weak    = ch->template weak_from_this_as<character>();
                            auto mail_id = mail.id;
                            auto builder = this->server.threads.new_builder(weak);
                            builder.func = [weak, mail_id](auto&) -> async::task<void> {
                                auto ptr = weak.lock();
                                if (ptr != nullptr)
                                    ptr->mail_box.mark_system_mail(mail_id);
                                co_return;
                            };
                            builder.enqueue();
                        }
                    }
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
