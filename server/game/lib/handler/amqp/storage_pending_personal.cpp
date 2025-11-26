#include <fb/game/handler/amqp/storage_pending_personal.h>
#include <fb/game/storage.h>
#include <fb/game/server.h>
#include <unordered_map>
#include <memory>
#include <vector>
#include <utility>

using namespace fb::game::handler::amqp;

storage_pending_personal::storage_pending_personal(fb::game::server& server) :
    fb::handler::amqp<fb::game::server, internal_resp::GetStoragePending>(server)
{ }

async::task<void> storage_pending_personal::handle(const internal_resp::GetStoragePending& message)
{
    if (message.user.has_value() == false)
        co_return;

    auto ch = this->server.characters.find(message.user.value());
    if (ch == nullptr)
        co_return;

    if (message.pending.empty())
        co_return;

    auto dao = std::vector<fb::game::storage_box::pending_box>{};
    for (const auto& dto : message.pending)
    {
        if (!dto.user.has_value())
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

    auto weak = ch->weak_from_this();
    co_await this->server.threads.switching(weak);
    ch->storage_box.apply_pending(dao);
}
