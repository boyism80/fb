#include <fb/game/channel/storage_pending_channel.h>
#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/logger.h>
#include <json/json.h>
#include <sstream>
#include <utility>

using namespace fb::game;

storage_pending_channel::storage_pending_channel(server& owner) :
    _owner(owner)
{ }

async::task<void> storage_pending_channel::fetch()
{
    try
    {
        auto&& resp = co_await this->_owner.http.get<fb::protocol::internal::response::GetStoragePending>("internal", "/storage/pending");
        if (resp.error != 0)
            co_return;

        this->_data.write([&resp](auto& buffer) {
            buffer.clear();

            for (const auto& dto : resp.pending)
            {
                storage_box::pending_box entry{};
                entry.id      = dto.id;
                entry.user    = dto.user;
                entry.title   = dto.title;
                entry.message = dto.message;

                if (!dto.attachments.empty())
                {
                    auto json   = Json::Value{};
                    auto reader = Json::Reader{};
                    auto stream = std::istringstream(dto.attachments);
                    if (reader.parse(stream, json) && json.isArray())
                    {
                        entry.attachments.reserve(json.size());
                        for (const auto& item : json)
                        {
                            entry.attachments.emplace_back(item);
                        }
                    }
                }

                if (dto.expired_date.has_value())
                    entry.expire_date = fb::model::datetime(dto.expired_date.value());

                buffer[entry.id] = std::move(entry);
            }
        });

        // Notify all connected characters about storage pending updates
        this->_owner.characters.write([this](auto& characters) {
            characters.foreach_enqueue([this](auto& ch) -> async::task<void> {
                this->_owner.storage_pending.read([&ch](const auto& pending_map) {
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
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("failed to fetch storage pending: {}", e.what());
    }

    co_return;
}

void storage_pending_channel::read(std::function<void(const std::unordered_map<std::string, storage_box::pending_box>&)> fn)
{
    this->_data.read(fn);
}

async::task<void> storage_pending_channel::read_async(std::function<async::task<void>(const std::unordered_map<std::string, storage_box::pending_box>&)> fn)
{
    co_await this->_data.async_read(fn);
}

void storage_pending_channel::write(std::function<void(std::unordered_map<std::string, storage_box::pending_box>&)> fn)
{
    this->_data.write(fn);
}

