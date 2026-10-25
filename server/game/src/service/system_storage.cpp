#include <fb/game/service/system_storage.h>

#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <json/json.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace fb::game;
namespace internal_resp = fb::protocol::internal::response;
namespace internal_reqs = fb::protocol::internal::request;

system_storage_box service::system_storage::from_system_storage_dto(const fb::protocol::internal::SystemStorageBox& dto)
{
    auto box = system_storage_box{
        .id           = dto.id,
        .user         = dto.user,
        .active       = dto.active,
        .title        = dto.title,
        .message      = dto.message,
        .attachments  = dto.attachments,
        .created_date = fb::model::datetime(dto.created_date),
    };

    if (dto.expired_date.has_value() && !dto.expired_date.value().empty())
        box.expire_date = fb::model::datetime(dto.expired_date.value());

    if (dto.external_ref.has_value() && !dto.external_ref.value().empty())
        box.external_ref = dto.external_ref.value();

    if (dto.created_before.has_value() && !dto.created_before.value().empty())
        box.created_before = fb::model::datetime(dto.created_before.value());

    return box;
}

storage_box::entry service::system_storage::from_storage_box_dto(const fb::protocol::internal::StorageBox& dto)
{
    auto box = storage_box::entry{};
    box.id   = dto.id;
    if (dto.system_storage_box_id != 0)
        box.system_storage_box_id = dto.system_storage_box_id;
    box.title   = dto.title;
    box.message = dto.message;

    if (!dto.attachments.empty())
    {
        auto json   = Json::Value{};
        auto reader = Json::Reader{};
        auto stream = std::istringstream(dto.attachments);
        if (reader.parse(stream, json) && json.isArray())
        {
            box.attachments.reserve(json.size());
            for (const auto& item : json)
            {
                box.attachments.emplace_back(item);
            }
        }
    }

    box.received = dto.received;
    if (dto.expired_date.has_value())
        box.expire_date = fb::model::datetime(dto.expired_date.value());

    return box;
}

void service::system_storage::prune_expired_boxes(std::vector<system_storage_box>& boxes,
                                                  const fb::model::datetime&       now)
{
    boxes.erase(std::remove_if(boxes.begin(),
                               boxes.end(),
                               [&](const system_storage_box& box) {
                                   return box.expired(now);
                               }),
                boxes.end());
}

std::string service::system_storage::attachments_to_json(const std::vector<fb::model::dsl>& attachments)
{
    auto json_array = Json::Value(Json::arrayValue);
    for (const auto& dsl : attachments)
    {
        json_array.append(dsl.to_json());
    }

    auto builder           = Json::StreamWriterBuilder{};
    builder["emitUTF8"]    = true;
    builder["indentation"] = "";
    auto writer            = std::unique_ptr<Json::StreamWriter>(builder.newStreamWriter());
    auto stream            = std::ostringstream{};
    writer->write(json_array, &stream);
    return stream.str();
}

service::system_storage::system_storage(fb::game::server& server) :
    server(server),
    _distribution(distribute_coroutine())
{ }

void service::system_storage::on_write(const fb::protocol::internal::SystemStorageBox& dto)
{
    if (dto.user == 0)
        return;

    this->join(dto.user);
}

void service::system_storage::init_from_login(character&                                             ch,
                                              const std::vector<fb::protocol::internal::StorageBox>& boxes,
                                              const std::vector<uint32_t>&                           archived)
{
    auto entries = std::vector<storage_box::entry>{};
    entries.reserve(boxes.size());
    for (const auto& dto : boxes)
    {
        entries.push_back(from_storage_box_dto(dto));
    }

    ch.storage_box.init(entries, archived);
}

async::task<void> service::system_storage::sync(character& ch)
{
    const auto  world = ch.world();
    const auto  user  = ch.id;
    const auto& url   = std::format("/storage/system/{}/{}", world, user);

    auto weak = ch.weak_from_this_as<character>();
    try
    {
        auto&& resp = co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>(weak, "internal", url);

        auto ptr = weak.lock();
        if (ptr == nullptr)
            co_return;

        if (resp.error != 0)
        {
            this->join(user);
            co_return;
        }

        const auto now     = this->server.now();
        auto       offered = std::vector<storage_box::entry>{};
        for (const auto& dto : resp.boxes)
        {
            auto box = from_system_storage_dto(dto);
            if (box.active && box.expired(now) == false && box.eligible(*ptr))
                offered.push_back(box.to_entry());
        }

        ptr->storage_box.add(offered);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("sync system storage failed for user {}: {}", user, e.what());
        this->join(user);
    }

    co_return;
}

async::task<bool> service::system_storage::create(uint32_t                           world,
                                                  std::string_view                   user_name,
                                                  std::string_view                   external_ref,
                                                  std::string_view                   title,
                                                  std::string_view                   message,
                                                  const std::vector<fb::model::dsl>& attachments,
                                                  const std::optional<std::string>&  expire_date)
{
    const auto name    = user_name.empty() ? std::nullopt : std::make_optional(std::string(user_name));
    const auto request = internal_reqs::WriteSystemStorageBox{world,
                                                              name,
                                                              std::string(title),
                                                              std::string(message),
                                                              attachments_to_json(attachments),
                                                              expire_date,
                                                              std::string(external_ref)};

    try
    {
        auto&& resp = co_await this->server.http.post("internal", "/storage/system", request);
        if (resp.error == 0)
            co_return true;

        fb::logger::warn("WriteSystemStorageBox failed for {} ({}): error {}", user_name, external_ref, resp.error);
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("WriteSystemStorageBox request failed for {} ({}): {}", user_name, external_ref, e.what());
    }
    co_return false;
}

async::task<void> service::system_storage::distribute()
{
    std::ignore = this->_distribution.next();
    co_return;
}

void service::system_storage::join(uint32_t user)
{
    auto _ = std::lock_guard(this->_joined_mutex);
    this->_joined.insert(user);
}

fb::async_generator<void> service::system_storage::distribute_coroutine()
{
    while (true)
    {
        auto joined = std::unordered_set<uint32_t>{};
        {
            auto _ = std::lock_guard(this->_joined_mutex);
            joined = std::exchange(this->_joined, {});
        }

        if (this->server.characters.size() == 0)
        {
            co_await fb::async_suspend{};
            continue;
        }

        auto new_box_seen = false;
        auto world        = fb::config<std::optional<uint32_t>>("world");
        if (world)
        {
            const auto now = this->server.now();
            prune_expired_boxes(this->_pending_boxes, now);

            // Internal returns every box that is sendable now. A reserved box becomes sendable later with an id
            // lower than boxes already seen, so the list is replaced instead of read from an offset.
            try
            {
                const auto& fetch_url = std::format("/storage/system/{}", *world);
                auto&&      resp =
                    co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>("internal", fetch_url);
                if (resp.error == 0)
                {
                    auto fetched = std::vector<system_storage_box>{};
                    for (const auto& dto : resp.boxes)
                    {
                        auto box = from_system_storage_dto(dto);
                        if (box.expired(now))
                            continue;

                        const auto already_tracked = std::any_of(this->_pending_boxes.cbegin(),
                                                                 this->_pending_boxes.cend(),
                                                                 [&](const system_storage_box& existing) {
                                                                     return existing.id == box.id;
                                                                 });
                        if (!already_tracked)
                            new_box_seen = true;

                        fetched.push_back(std::move(box));
                    }
                    this->_pending_boxes = std::move(fetched);
                }
            }
            catch (const std::exception& e)
            {
                fb::logger::warn("Failed to fetch system storage boxes: {}", e.what());
            }
        }

        if (new_box_seen)
        {
            co_await this->server.characters.foreach_async([this](auto& ch) -> async::task<void> {
                co_await this->sync(*ch);
            });
        }
        else if (joined.empty() == false)
        {
            auto targets = std::vector<std::shared_ptr<character>>{};
            for (auto user : joined)
            {
                auto ch = this->server.characters.find(user);
                if (ch != nullptr)
                    targets.push_back(ch);
            }
            co_await this->server.characters.foreach_async(
                [this](auto& ch) -> async::task<void> {
                    co_await this->sync(*ch);
                },
                targets);
        }

        co_await fb::async_suspend{};
    }
}
