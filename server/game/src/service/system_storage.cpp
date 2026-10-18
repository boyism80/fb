#include <fb/game/service/system_storage.h>

#include <fb/amqp_route.h>
#include <fb/config.h>
#include <fb/game/character.h>
#include <fb/game/server.h>
#include <fb/logger.h>
#include <fb/protocol/flatbuffer/protocol.h>

#include <json/json.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
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

void service::system_storage::add(const std::vector<storage_box::entry>& entries, const std::vector<uint32_t>& user_ids)
{
    if (entries.empty() || entries.size() != user_ids.size())
        return;

    for (size_t i = 0; i < entries.size(); ++i)
    {
        auto ch = this->server.characters.find(user_ids[i]);
        if (ch == nullptr)
            continue;

        auto weak    = ch->template weak_from_this_as<character>();
        auto entry   = entries[i];
        auto builder = this->server.threads.new_builder(weak);
        builder.func = [weak, entry](auto&) -> async::task<void> {
            auto ptr = weak.lock();
            if (ptr == nullptr)
                co_return;

            if (entry.system_storage_box_id.has_value() &&
                ptr->storage_box.contains_system_box(entry.system_storage_box_id.value()))
                co_return;

            ptr->storage_box.add({entry});
            co_return;
        };
        builder.enqueue();
    }
}

void service::system_storage::on_write_box(const fb::protocol::internal::StorageBox& dto)
{
    auto entry = from_storage_box_dto(dto);
    auto user  = dto.user != 0 ? dto.user : 0u;
    if (user == 0)
        return;

    this->add({entry}, {user});
}

void service::system_storage::on_distributed(const std::vector<fb::protocol::internal::StorageWriteEntry>& entries)
{
    auto converted = std::vector<storage_box::entry>{};
    auto user_ids  = std::vector<uint32_t>{};
    converted.reserve(entries.size());
    user_ids.reserve(entries.size());

    for (const auto& entry : entries)
    {
        const auto& box     = entry.box;
        const auto  user_id = box.user != 0 ? box.user : entry.user;
        user_ids.push_back(user_id);
        converted.push_back(from_storage_box_dto(box));
    }

    this->add(converted, user_ids);
}

void service::system_storage::init_from_login(character&                                             ch,
                                              const std::vector<fb::protocol::internal::StorageBox>& boxes)
{
    auto entries = std::vector<storage_box::entry>{};
    entries.reserve(boxes.size());
    for (const auto& dto : boxes)
    {
        entries.push_back(from_storage_box_dto(dto));
    }

    ch.storage_box.init(entries);
}

async::task<void> service::system_storage::sync(character& ch)
{
    const auto  world = ch.world();
    const auto  user  = ch.id;
    const auto& url   = std::format("/storage/system/{}", world);

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

        const auto now = this->server.now();
        auto       ids = std::vector<uint32_t>{};

        for (const auto& dto : resp.boxes)
        {
            auto box = from_system_storage_dto(dto);
            if (box.expired(now))
                continue;

            if (!box.eligible_for(*ptr))
                continue;

            if (ptr->storage_box.contains_system_box(box.id))
                continue;

            try
            {
                auto&& distribute_resp = co_await this->server.http.post(
                    weak,
                    "internal",
                    "/storage/system/distribute",
                    internal_reqs::DistributeSystemStorage{world, box.id, {ptr->id}, fb::config<uint32_t>("id")});
                if (distribute_resp.error != 0)
                {
                    fb::logger::warn("DistributeSystemStorage sync failed for box {}: error {}",
                                     box.id,
                                     distribute_resp.error);
                    this->join(ptr->id);
                }
                else
                {
                    this->on_distributed(distribute_resp.entries);
                }
            }
            catch (const std::exception& e)
            {
                fb::logger::warn("DistributeSystemStorage sync request failed for box {}: {}", box.id, e.what());
                this->join(ptr->id);
            }
        }
    }
    catch (const std::exception& e)
    {
        fb::logger::warn("sync system storage failed for user {}: {}", user, e.what());
        this->join(user);
    }

    co_return;
}

async::task<bool> service::system_storage::create(uint32_t                           world,
                                                  uint32_t                           user_id,
                                                  std::string_view                   external_ref,
                                                  std::string_view                   title,
                                                  std::string_view                   message,
                                                  const std::vector<fb::model::dsl>& attachments,
                                                  const std::optional<std::string>&  expire_date)
{
    const auto request = internal_reqs::WriteStorageBox{world,
                                                        user_id,
                                                        std::nullopt,
                                                        std::string(title),
                                                        std::string(message),
                                                        attachments_to_json(attachments),
                                                        expire_date,
                                                        std::string(external_ref),
                                                        fb::config<uint32_t>("id")};

    // The external ref makes a resend return the box an earlier attempt already wrote.
    for (auto attempt = 1; attempt <= write_attempts; attempt++)
    {
        try
        {
            auto&& resp = co_await this->server.http.post("internal", "/storage/write", request);
            if (resp.error == 0)
                co_return true;

            fb::logger::warn("WriteStorageBox failed for {} (attempt {}): error {}", external_ref, attempt, resp.error);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("WriteStorageBox request failed for {} (attempt {}): {}", external_ref, attempt, e.what());
        }
    }
    co_return false;
}

async::task<bool> service::system_storage::create(uint32_t                           world,
                                                  std::string_view                   user_name,
                                                  std::string_view                   external_ref,
                                                  std::string_view                   title,
                                                  std::string_view                   message,
                                                  const std::vector<fb::model::dsl>& attachments,
                                                  const std::optional<std::string>&  expire_date)
{
    const auto request = internal_reqs::WriteStorageBox{world,
                                                        0,
                                                        std::string(user_name),
                                                        std::string(title),
                                                        std::string(message),
                                                        attachments_to_json(attachments),
                                                        expire_date,
                                                        std::string(external_ref),
                                                        fb::config<uint32_t>("id")};

    for (auto attempt = 1; attempt <= write_attempts; attempt++)
    {
        try
        {
            auto&& resp = co_await this->server.http.post("internal", "/storage/write", request);
            if (resp.error == 0)
                co_return true;

            fb::logger::warn("WriteStorageBox failed for name {} ({}, attempt {}): error {}",
                             user_name,
                             external_ref,
                             attempt,
                             resp.error);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("WriteStorageBox request failed for name {} ({}, attempt {}): {}",
                             user_name,
                             external_ref,
                             attempt,
                             e.what());
        }
    }
    co_return false;
}

async::task<bool> service::system_storage::create_system(uint32_t                           world,
                                                         std::string_view                   external_ref,
                                                         std::string_view                   title,
                                                         std::string_view                   message,
                                                         const std::vector<fb::model::dsl>& attachments,
                                                         const std::optional<std::string>&  expire_date)
{
    const auto request = internal_reqs::WriteSystemStorageBox{world,
                                                              std::string(title),
                                                              std::string(message),
                                                              attachments_to_json(attachments),
                                                              expire_date,
                                                              std::string(external_ref)};

    for (auto attempt = 1; attempt <= write_attempts; attempt++)
    {
        try
        {
            auto&& resp = co_await this->server.http.post("internal", "/storage/system", request);
            if (resp.error == 0)
                co_return true;

            fb::logger::warn("WriteSystemStorageBox failed for {} (attempt {}): error {}",
                             external_ref,
                             attempt,
                             resp.error);
        }
        catch (const std::exception& e)
        {
            fb::logger::warn("WriteSystemStorageBox request failed for {} (attempt {}): {}",
                             external_ref,
                             attempt,
                             e.what());
        }
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
        auto world = fb::config<std::optional<uint32_t>>("world");
        if (!world)
        {
            co_await fb::async_suspend{};
            continue;
        }

        const auto now = this->server.now();
        prune_expired_boxes(this->_pending_boxes, now);

        if (this->server.characters.size() == 0)
        {
            co_await fb::async_suspend{};
            continue;
        }

        auto        new_box_seen = false;
        const auto& fetch_url    = std::format("/storage/system/{}", *world);

        // Internal returns every box that is sendable now. A reserved box becomes sendable later with an id
        // lower than boxes already seen, so the list is replaced instead of read from an offset.
        try
        {
            auto&& resp = co_await this->server.http.get<internal_resp::GetSystemStorageBoxes>("internal", fetch_url);
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

        auto joined = std::unordered_set<uint32_t>{};
        {
            auto _ = std::lock_guard(this->_joined_mutex);
            joined = std::exchange(this->_joined, {});
        }

        auto pending_boxes = std::make_shared<std::vector<system_storage_box>>(this->_pending_boxes);
        auto recipients    = std::make_shared<std::unordered_map<uint32_t, std::vector<uint32_t>>>();
        auto mutex         = std::make_shared<std::mutex>();
        auto collect       = [pending_boxes, recipients, mutex](auto& ch) -> async::task<void> {
            for (const auto& box : *pending_boxes)
            {
                if (box.eligible_for(*ch) && !ch->storage_box.contains_system_box(box.id))
                {
                    auto _ = std::lock_guard(*mutex);
                    (*recipients)[box.id].push_back(ch->id);
                }
            }
            co_return;
        };

        // Old boxes are only rechecked for characters in _joined, so every path that can leave an
        // online character without a pending box (failed login sync, failed distribute) must call join().
        if (new_box_seen)
        {
            co_await this->server.characters.foreach_async(collect);
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
            co_await this->server.characters.foreach_async(collect, targets);
        }

        for (const auto& box : this->_pending_boxes)
        {
            if (box.expired(now))
                continue;

            auto found = recipients->find(box.id);
            if (found == recipients->end())
                continue;

            const auto& eligible = found->second;

            for (std::size_t i = 0; i < eligible.size(); i += chunk_limit)
            {
                const auto end         = std::min(i + chunk_limit, eligible.size());
                auto       chunk_users = std::vector<uint32_t>{};
                chunk_users.assign(eligible.begin() + static_cast<std::ptrdiff_t>(i),
                                   eligible.begin() + static_cast<std::ptrdiff_t>(end));
                const auto chunk_users_to_join = chunk_users;

                try
                {
                    auto&& resp = co_await this->server.http.post(
                        "internal",
                        "/storage/system/distribute",
                        internal_reqs::DistributeSystemStorage{*world,
                                                               box.id,
                                                               std::move(chunk_users),
                                                               fb::config<uint32_t>("id")});
                    if (resp.error != 0)
                    {
                        fb::logger::warn("DistributeSystemStorage failed for box {}: error {}", box.id, resp.error);
                        for (auto user_id : chunk_users_to_join)
                        {
                            this->join(user_id);
                        }
                    }
                    else
                    {
                        this->on_distributed(resp.entries);
                    }
                }
                catch (const std::exception& e)
                {
                    fb::logger::warn("DistributeSystemStorage request failed for box {}: {}", box.id, e.what());
                    for (auto user_id : chunk_users_to_join)
                    {
                        this->join(user_id);
                    }
                }
            }
        }

        co_await fb::async_suspend{};
    }
}
