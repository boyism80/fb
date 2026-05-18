#include <fb/game/handler/timer/system_storage_box_timer.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/system_storage_box.h>
#include <fb/logger.h>
#include <algorithm>
#include <format>

using namespace fb::game::handler::timer;

namespace internal_resp = fb::protocol::internal::response;

bool system_storage_box_timer::is_expired(const fb::game::system_storage_box& box, const fb::model::datetime& now)
{
    return box.expire_date.has_value() && box.expire_date.value() < now;
}

void system_storage_box_timer::prune_expired_boxes(std::vector<fb::game::system_storage_box>& boxes,
                                                   const fb::model::datetime&                 now)
{
    boxes.erase(std::remove_if(boxes.begin(),
                               boxes.end(),
                               [&](const fb::game::system_storage_box& box) {
                                   return is_expired(box, now);
                               }),
                boxes.end());
}

system_storage_box_timer::system_storage_box_timer(fb::game::server& server) :
    fb::handler::timer<fb::game::server>(server),
    _delivery{delivery_coroutine()}
{ }

async::task<void> system_storage_box_timer::handle()
{
    std::ignore = co_await this->_delivery.next();
    co_return;
}

std::vector<std::tuple<uint32_t, fb::model::datetime>> system_storage_box_timer::collect_online_users() const
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

fb::async_generator<void> system_storage_box_timer::delivery_coroutine()
{
    auto offset = uint32_t{0};
    auto boxes  = std::vector<fb::game::system_storage_box>{};

    while (true)
    {
        auto        max_box_id = uint32_t{0};
        const auto  world      = fb::config<uint32_t>("world");
        const auto& fetch_url  = std::format("/storage/system/{}?offset={}", world, offset);

        try
        {
            auto&& resp = co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>("internal", fetch_url);
            if (resp.error == 0)
            {
                const auto now = this->server.now();
                for (const auto& dto : resp.boxes)
                {
                    auto box = fb::game::system_storage_box(dto);
                    if (is_expired(box, now))
                        continue;

                    max_box_id = std::max(max_box_id, box.id);

                    const auto already_tracked =
                        std::any_of(boxes.cbegin(), boxes.cend(), [&](const fb::game::system_storage_box& existing) {
                            return existing.id == box.id;
                        });
                    if (!already_tracked)
                        boxes.push_back(std::move(box));
                }
            }
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("Failed to fetch system storage boxes: {}", e.what());
        }

        if (max_box_id > 0)
            offset = max_box_id + 1;

        const auto now          = this->server.now();
        const auto online_users = this->collect_online_users();
        prune_expired_boxes(boxes, now);

        for (const auto& box : boxes)
        {
            if (is_expired(box, now))
                continue;

            if (box.user.has_value())
            {
                this->server.apply_system_storage_to_users({box.user.value()}, box);
            }
            else
            {
                auto eligible = std::vector<uint32_t>{};
                eligible.reserve(online_users.size());

                for (const auto& [user_id, created_date] : online_users)
                {
                    if (created_date < box.created_date)
                        eligible.push_back(user_id);
                }

                this->server.apply_system_storage_to_users(eligible, box);
            }
        }

        co_await fb::async_suspend{};
    }
}
