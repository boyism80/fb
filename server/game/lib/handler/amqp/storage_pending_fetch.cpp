#include <fb/game/handler/amqp/storage_pending_fetch.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <json/json.h>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <utility>
#include <iterator>

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

        auto pending    = fb::game::storage_box::pending_box{};
        pending.id      = dto.id;
        pending.user    = dto.user;
        pending.title   = dto.title;
        pending.message = dto.message;

        if (!dto.attachments.empty())
        {
            auto json   = Json::Value{};
            auto reader = Json::Reader{};
            auto stream = std::istringstream(dto.attachments);
            if (reader.parse(stream, json) && json.isArray())
            {
                pending.attachments.reserve(json.size());
                for (const auto& item : json)
                {
                    pending.attachments.emplace_back(item);
                }
            }
        }

        if (dto.expired_date.has_value())
            pending.expire_date = fb::model::datetime(dto.expired_date.value());

        dao.push_back(std::move(pending));
    }

    if (dao.empty())
        co_return;

    this->server.storage_pending.write([&dao](auto& buffer) {
        // Append only non-duplicate items (map automatically handles duplicates by key)
        for (auto& item : dao)
        {
            if (buffer.find(item.id) == buffer.end())
            {
                buffer[item.id] = std::move(item);
            }
        }
    });

    // Notify all connected characters about new storage pending
    auto& server = this->server;
    server.characters.write([&server](auto& characters) {
        characters.foreach_enqueue([&server](auto& ch) -> async::task<void> {
            server.storage_pending.read([&ch](const auto& pending_map) {
                if (pending_map.empty())
                    return;

                // Convert map to vector for apply_pending
                auto pending = std::vector<fb::game::storage_box::pending_box>();
                pending.reserve(pending_map.size());
                for (const auto& [id, box] : pending_map)
                {
                    pending.push_back(box);
                }

                ch->storage_box.apply_pending(pending);
            });
            co_return;
        });
    });

    co_return;
}
