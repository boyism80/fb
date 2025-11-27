#include <fb/game/server.h>
#include <fb/game/storage.h>
#include <fb/model/model.h>
#include <fb/logger.h>
#include <json/json.h>
#include <sstream>

using namespace fb::game;

async::task<void> server::fetch_storage_pending()
{
    try
    {
        auto&& resp = co_await this->http.get<internal_resp::GetStoragePending>("internal", "/storage/pending");
        if (resp.error != 0)
            co_return;

        auto pending = std::vector<fb::game::storage_box::pending_box>();
        pending.reserve(resp.pending.size());

        for (const auto& dto : resp.pending)
        {
            fb::game::storage_box::pending_box entry{};
            entry.id      = dto.id;
            entry.user    = dto.user;
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

            pending.push_back(std::move(entry));
        }

        this->_storage_pending.write([&pending](auto& buffer) {
            buffer = std::move(pending);
        });
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("failed to fetch storage pending: {}", e.what());
    }

    co_return;
}

void server::read_storage_pending(std::function<void(const std::vector<fb::game::storage_box::pending_box>&)> fn)
{
    this->_storage_pending.read(fn);
}

async::task<void> server::read_storage_pending_async(std::function<async::task<void>(const std::vector<fb::game::storage_box::pending_box>&)> fn)
{
    co_await this->_storage_pending.async_read(fn);
}

void server::write_storage_pending(std::function<void(std::vector<fb::game::storage_box::pending_box>&)> fn)
{
    this->_storage_pending.write(fn);
}