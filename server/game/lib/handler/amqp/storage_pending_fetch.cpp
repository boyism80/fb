#include <fb/game/handler/amqp/storage_pending_fetch.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>

using namespace fb::game::handler::amqp;

storage_pending_fetch::storage_pending_fetch(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::GetStoragePending>(server)
{ }

async::task<void> storage_pending_fetch::handle(const internal_resp::GetStoragePending& message)
{
    if (message.user.has_value())
        co_return;

    if (message.pending.empty())
        co_return;

    auto dao = std::vector<fb::game::storage_box::pending_box>{};
    for (const auto& dto : message.pending)
    {
        if (dto.user.has_value())
            continue;

        auto pending        = fb::game::storage_box::pending_box{};
        pending.id          = dto.id;
        pending.user        = dto.user;
        pending.message     = dto.message;
        pending.attachments = dto.attachments;
        if (dto.expired_date.has_value())
            pending.expire_date = fb::model::datetime(dto.expired_date.value());

        dao.push_back(std::move(pending));
    }

    if (dao.empty())
        co_return;

    this->server.write_storage_pending([&dao](auto& buffer) {
        buffer.reserve(buffer.size() + dao.size());
        buffer.insert(buffer.end(), dao.begin(), dao.end());
    });

    co_return;
}
